/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software must only be used in or with a processor manufactured by Nordic
 *    Semiconductor ASA, or in or with a processor manufactured by a third party that
 *    is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* PURPOSE: Transport-independent interface between LL and HL for NCP.
*/
#define ZB_TRACE_FILE_ID 2
#include "zb_common.h"
#include "ncp/zb_ncp_ll_dev.h"
#include "transport/zb_ncp_tr.h"
#include "low_level/zbncp.h"

/* Synchronize ZB callback/alarm scheduling between main ZBOSS task
   and low-level protocol callback possibly issued from interrupt
   context. Implement this synchronization in a semaphore-like manner.
   The signal_count field indicates how many times the "semaphore"
   has been signaled. It is modified under global lock so there is
   no race condition possible. The wait_count field indicates how
   many times we waited for the "semaphore". It is modified only
   in the wait routine, and enables us to understand if we need to
   schedule next alarm/callback or it was already scheduled by the
   signal routine. If the difference between the counters is zero -
   we returned to the quantum execution after the previously scheduled
   alarm timed out, and the semaphore had not been signaled in between.
   If the difference equals to one - we were executed because the
   the signal routine scheduled one callback, but in order to continue
   scheduling loop we must schedule next callback/alarm ourselves.
   Otherwise we shouldn't do anything because there is at least one
   callback pending, which has been scheduled by the signal routine
   previously. */

typedef struct ncp_ll_sch_sem_s
{
  volatile long int signal_count;
  long int wait_count;
  zb_callback_t cb;
  zb_uint8_t arg;
}
ncp_ll_sch_sem_t;

static void ncp_ll_sch_sem_init(ncp_ll_sch_sem_t *sem, zb_callback_t cb, zb_uint8_t arg)
{
  sem->signal_count = 0u;
  sem->wait_count = 0u;
  sem->cb = cb;
  sem->arg = arg;
}

static void ncp_ll_sch_sem_signal(ncp_ll_sch_sem_t *sem)
{
  zb_ret_t err_code = ZB_SCHEDULE_APP_CALLBACK(sem->cb, sem->arg);

  ZB_OSIF_GLOBAL_LOCK();

  /* Modify signal count under global lock because the signal routine
     can be called either from interrupt context or from the main
     task scheduling context in the packet sending routine. */
  if (err_code == RET_OK)
  {
    ++sem->signal_count;
  }

  ZB_OSIF_GLOBAL_UNLOCK();
}

static void ncp_ll_sch_sem_wait(ncp_ll_sch_sem_t *sem, zbncp_ll_time_t timeout)
{
  zb_bool_t need_alarm = ZB_FALSE;
  long int pending_count;

  ZB_SCHEDULE_APP_ALARM_CANCEL(sem->cb, sem->arg);

  ZB_OSIF_GLOBAL_LOCK();

  /* Read signal count under global lock to avoid race conditions. */
  pending_count = sem->signal_count - sem->wait_count;

  ZB_OSIF_GLOBAL_UNLOCK();

  /* Do not compare signal_count and wait_count directly
     as they can overflow. Subtracting will properly handle
     overflow if the difference is not too large. */
  if (pending_count > 0)
  {
    /* We have been woken up by the callback scheduled in 'call_me' */

    /* Acknowledge callback */
    ++sem->wait_count;

    /* If we are in the context of the last scheduled callback then we
       need to schedule next alarm ourselves */
    if (pending_count == 1)
    {
      need_alarm = ZB_TRUE;
    }
  }
  else if (timeout != ZBNCP_LL_TIMEOUT_INFINITE)
  {
    /* We have been woken up by timed-out alarm */
    need_alarm = ZB_TRUE;
  }

  TRACE_MSG(TRACE_USB3, "pending_count: %d need_alarm: %d", (FMT__D_D, pending_count, need_alarm));

  if (need_alarm)
  {
    /* Note: if timeout == 0, ZB_SCHEDULE_APP_ALARM optimizes to cb */
    ZB_SCHEDULE_APP_ALARM(sem->cb, sem->arg, ZB_MILLISECONDS_TO_BEACON_INTERVAL(timeout));
  }
  else
  {
    /* Callback has been already scheduled */
  }
}

typedef struct ncp_ll_ctx_s
{
  ncp_ll_packet_received_cb_t rx_packet_cb;
  ncp_ll_tx_ready_cb_t tx_ready_cb;
  zbncp_ll_proto_t *ll;
  ncp_ll_sch_sem_t sch_sem;
  zb_uint8_t rxbuf[ZBNCP_BIG_BUF_SIZE];
}
ncp_ll_ctx_t;

static ncp_ll_ctx_t ncp_ll_ctx;

static zbncp_frag_ctx_t frag_ctx;

static void ncp_ll_call_me_cb(zbncp_ll_cb_arg_t arg)
{
  ncp_ll_sch_sem_signal(&ncp_ll_ctx.sch_sem);
  ZVUNUSED(arg);
}

/* Note: all the following routines run in the main ZBOSS loop. */

static inline zbncp_ll_time_t ncp_ll_get_time_msec(void)
{
  return ZBNCP_USECS_TO_MILLISECONDS(osif_transceiver_time_get_long()); /* Convert to milliseconds */
}

void ncp_ll_proto_init(ncp_ll_packet_received_cb_t packet_received_cb, ncp_ll_tx_ready_cb_t tx_ready_cb)
{
  const zbncp_transport_ops_t *tr_ops = ncp_host_transport_create();
  zbncp_ll_proto_cb_t llcb;

  TRACE_MSG(TRACE_USB3, ">ncp_ll_proto_init packet_received_cb %p tx_ready_cb %p", (FMT__P_P, packet_received_cb, tx_ready_cb));

  ncp_ll_ctx.rx_packet_cb = packet_received_cb;
  ncp_ll_ctx.tx_ready_cb = tx_ready_cb;
  ncp_ll_ctx.ll = zbncp_ll_create(tr_ops);
  ncp_ll_sch_sem_init(&ncp_ll_ctx.sch_sem, ncp_ll_quant, ZB_ALARM_ANY_PARAM);

  llcb.callme = ncp_ll_call_me_cb;
  llcb.arg = ZBNCP_NULL;
  zbncp_ll_init(ncp_ll_ctx.ll, &llcb, ncp_ll_get_time_msec());

  zbncp_frag_initialize(&frag_ctx);

  TRACE_MSG(TRACE_USB3, "<ncp_ll_proto_init", (FMT__0));
}


/**
   Send packet to LL protocol.

   To be called from HL.  Note that HL logic itself can be called from
   ncp_ll_quant -> rx_packet_cb, so this routine takes care of
   preventing infinite recursion.
 */
zb_int_t ncp_ll_send_packet(void *data, zb_uint32_t len)
{
  zb_int_t ret = RET_OK;

  TRACE_MSG(TRACE_USB3, ">ncp_ll_send_packet data %p len %ld", (FMT__P_L, data, (zb_uint_t) len));
  ret = zbncp_frag_store_tx_pkt(&frag_ctx, data, len);
  if (RET_OK == ret)
  {
    /* Do not call ncp_ll_quant() directly to prevent a recursion. */
    ncp_ll_sch_sem_signal(&ncp_ll_ctx.sch_sem);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Oops, TX mem ptr is still valid from previous call, status %hd", (FMT__H, ret));
    ret = RET_BUSY;
  }
  TRACE_MSG(TRACE_USB3, "<ncp_ll_send_packet", (FMT__0));

  return ret;
}


void ncp_ll_quant(zb_uint8_t unused)
{
  zbncp_ll_quant_t llq;
  zbncp_size_t rx_size = 0;
  zbncp_bool_t tx_ready = ZBNCP_FALSE;

  ZVUNUSED(unused);

  TRACE_MSG(TRACE_USB3, ">ncp_ll_quant big buf size %ld", (FMT__D, (zb_uint_t) ZBNCP_BIG_BUF_SIZE));

  zbncp_frag_set_place_for_rx_pkt(&frag_ctx, ncp_ll_ctx.rxbuf, sizeof(ncp_ll_ctx.rxbuf));

  llq.req.time = ncp_ll_get_time_msec();
  zbncp_frag_fill_request(&frag_ctx, &llq.req);

  zbncp_ll_poll(ncp_ll_ctx.ll, &llq);

  rx_size = zbncp_frag_process_rx_response(&frag_ctx, &llq.res);
  if (rx_size > 0)
  {
    /* Pass packet to HL */
    if (ncp_ll_ctx.rx_packet_cb)
    {
      ncp_ll_ctx.rx_packet_cb(ncp_ll_ctx.rxbuf, (zb_uint16_t)rx_size);
    }
  }

  tx_ready = zbncp_frag_process_tx_response(&frag_ctx, &llq.res);
  if (tx_ready)
  {
    /* Notify HL */
    if (ncp_ll_ctx.tx_ready_cb)
    {
      ncp_ll_ctx.tx_ready_cb();
    }
  }

  TRACE_MSG(TRACE_USB3, "<ncp_ll_quant rx %ld tx %ld timeout %ld", (FMT__D_D_D, (zb_uint_t) llq.res.rx_info.rxbytes, (zb_uint_t) llq.res.txbytes, (zb_uint_t) llq.res.timeout));

  ncp_ll_sch_sem_wait(&ncp_ll_ctx.sch_sem, llq.res.timeout);
}
