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
/* PURPOSE: abstract async I/O
*/

#define ZB_TRACE_FILE_ID 30014
#include "zb_common.h"

#define ZB_TRANSPORT_MAX_HANDLER_CNT 5

OSIF_DECLARE_IPC_WAIT_CONTROL_T(platform_wait_control_t, ZB_TRANSPORT_MAX_HANDLER_CNT);

typedef struct transport_ctx_s
{
  platform_wait_control_t wc;
  osif_io_handler_t handlers[ZB_TRANSPORT_MAX_HANDLER_CNT];
  zb_uindex_t handle_cnt;
} transport_ctx_t;


static transport_ctx_t transport_ctx;


static zb_bool_t is_handler_record_empty(zb_uindex_t index)
{
  return transport_ctx.handlers[index] == NULL;
}


static void write_handler_record(zb_uindex_t index,
                                 osif_ipc_handle_t handle,
                                 zb_uint8_t event_mask,
                                 osif_io_handler_t handler)
{
  ZB_ASSERT(is_handler_record_empty(index));

  transport_ctx.handle_cnt++;
  osif_ipc_setup_wait(transport_ctx.wc, index, handle, event_mask);
  transport_ctx.handlers[index] = handler;
}


static void free_handler_record(zb_uindex_t index)
{
  ZB_ASSERT(!is_handler_record_empty(index));

  transport_ctx.handle_cnt--;
  osif_ipc_remove_wait(transport_ctx.wc, index);
  transport_ctx.handlers[index] = NULL;
}


static zb_ret_t find_empty_record(zb_uindex_t *index)
{
  zb_ret_t ret = RET_NO_MEMORY;
  zb_uindex_t i = 0;

  for (i = 0; i < ZB_TRANSPORT_MAX_HANDLER_CNT; i++)
  {
    if (is_handler_record_empty(i))
    {
      *index = i;
      ret = RET_OK;
      break;
    }
  }

  return ret;
}


static zb_ret_t find_handler_record_by_handle(osif_ipc_handle_t handle, zb_uindex_t *index)
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_uindex_t i = 0;

  for (i = 0; i < ZB_TRANSPORT_MAX_HANDLER_CNT; i++)
  {
    if (osif_ipc_get_handler(transport_ctx.wc, i) == handle)
    {
      *index = i;
      ret = RET_OK;
      break;
    }
  }

  return ret;
}


static zb_uint_t calculate_select_timeout(void)
{
  zb_uint_t ret = NCP_TRANSPORT_REFRESH_TIME;
  zb_time_t timeout_bi = 0; /*!< timeout in the internal ZBOSS time units  */

  TRACE_MSG(TRACE_OSIF4, ">> calculate_select_timeout", (FMT__0));

  TRACE_MSG(TRACE_OSIF4, "started %d stop %d timer %d",
            (FMT__D_D_D, ZB_TIMER_CTX().started, ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer));

  if (ZB_TIMER_CTX().started) /* if have any alarm running  */
  {
    /* if it is not time to run alarm already */
    if (ZB_TIME_GE(ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer)
        && ZB_TIMER_CTX().timer_stop != ZB_TIMER_CTX().timer)
    {
      timeout_bi = ZB_TIME_SUBTRACT(ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer);
      TRACE_MSG(TRACE_OSIF4, "timer stop %d timer %d timeout_bi %d",
                (FMT__D_D_D, ZB_TIMER_CTX().timer_stop, ZB_TIMER_CTX().timer, timeout_bi));

      ret = ZB_TIME_BEACON_INTERVAL_TO_MSEC(timeout_bi) * 1000;
    }
    else
    {
      /* zero timeout (no wait) */
      ret = 0;
    }
  }

  /* Maximum sleep time is limited by the NCP_TRANSPORT_REFRESH_TIME value. */
  if ((NCP_TRANSPORT_REFRESH_TIME > 0) && (ret > NCP_TRANSPORT_REFRESH_TIME))
  {
    ret = NCP_TRANSPORT_REFRESH_TIME;
  }

  TRACE_MSG(TRACE_OSIF4, "<< calculate_select_timeout, ret %d", (FMT__D, ret));
  return ret;
}

static void call_handlers(void)
{
  zb_uindex_t i = 0;
  zb_uint_t event_mask = 0;

  TRACE_MSG(TRACE_OSIF4, ">> call_handlers", (FMT__0));

  for (i = 0; i < ZB_TRANSPORT_MAX_HANDLER_CNT; i++)
  {
    if (!is_handler_record_empty(i)
        && osif_ipc_signaled(transport_ctx.wc, i, &event_mask))
    {
      TRACE_MSG(TRACE_OSIF4, "i/o event, handle %d, event mask 0x%x",
                (FMT__D_D, osif_ipc_get_handler(transport_ctx.wc, i), event_mask));

      if (transport_ctx.handlers[i])
      {
        TRACE_MSG(TRACE_OSIF4, "calling cb %p", (FMT__P, transport_ctx.handlers[i]));
        transport_ctx.handlers[i](event_mask);
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "handler is not set for i/o event, handle %d",
                  (FMT__D, osif_ipc_get_handler(transport_ctx.wc, i)));
      }
    }
  }

  TRACE_MSG(TRACE_OSIF4, "<< call_handlers", (FMT__0));
}


void zb_osif_platform_io_iteration(zb_bool_t block)
{
  zb_uint_t select_timeout = 0;
  zb_int_t wait_status = 0;

  TRACE_MSG(TRACE_OSIF4, ">> zb_osif_platform_io_iteration, block %d", (FMT__D, block));

  zb_osif_update_timer();

  if (block)
  {
    select_timeout = calculate_select_timeout();
  }

  if (transport_ctx.handle_cnt)
  {
    wait_status = osif_ipc_wait_for_io(transport_ctx.wc, transport_ctx.handle_cnt, select_timeout);
    if (wait_status < 0)
    {
      TRACE_MSG(TRACE_ERROR, "osif_ipc_wait_for_io error: %d", (FMT__D, wait_status));
    }
    else if (wait_status > 0)
    {
      call_handlers();
    }
  }
  else
  {
    /* if no transport is registered, we still need to sleep to avoid busy looping */
    osif_usleep(select_timeout);
  }

  zb_osif_update_timer();

  TRACE_MSG(TRACE_OSIF4, "<< zb_osif_platform_io_iteration", (FMT__0));
}


#ifdef ZB_USE_SLEEP
zb_uint32_t zb_osif_sleep(zb_uint32_t sleep_tmo)
{
  zb_int_t wait_status = 0;

  if (transport_ctx.handle_cnt)
  {
    wait_status = osif_ipc_wait_for_io(transport_ctx.wc, transport_ctx.handle_cnt, sleep_tmo * 1000LL);
    if (wait_status < 0)
    {
      TRACE_MSG(TRACE_ERROR, "osif_ipc_wait_for_io error: %d", (FMT__D, wait_status));
    }
    else if (wait_status > 0)
    {
      call_handlers();
    }
    /* If osif_ipc_wait_for_io returns zero - there is no event and the timeout has elapsed. */
  }
  else
  {
    /* if no transport is registered, we still need to sleep to avoid busy looping */
    osif_usleep(sleep_tmo * 1000LL);
  }

  zb_osif_update_timer();
  return sleep_tmo;
}


void zb_osif_wake_up()
{
}
#endif


static zb_bool_t is_duplicate(osif_ipc_handle_t handle,
                              osif_io_handler_t handler)
{
  zb_ret_t ret = ZB_FALSE;
  zb_uindex_t i;

  for (i = 0; i < transport_ctx.handle_cnt; i++)
  {
    if (osif_ipc_get_handler(transport_ctx.wc, i) == handle
        && transport_ctx.handlers[i] == handler)
    {
      ret = ZB_TRUE;
      break;
    }
  }

  return ret;
}


zb_ret_t osif_transport_add_io_handler(osif_ipc_handle_t handle,
                                       zb_uint8_t event_mask,
                                       osif_io_handler_t handler)
{
  zb_ret_t ret = RET_OK;
  zb_uindex_t record_index;

  TRACE_MSG(TRACE_OSIF2, ">> osif_transport_add_io_handler, "
            "handle %d, event_mask 0x%x, handler %p",
            (FMT__D_D_P, handle, event_mask, handler));

  ZB_ASSERT(handle != OSIF_IPC_INVALID_H);
  ZB_ASSERT(event_mask);
  ZB_ASSERT(handler);

  if (is_duplicate(handle, handler))
  {
    ret = RET_ALREADY_EXISTS;
  }
  else
  {
    ret = find_empty_record(&record_index);

    if (ret == RET_OK)
    {
      write_handler_record(record_index, handle, event_mask, handler);
    }
  }

  TRACE_MSG(TRACE_OSIF2, "<< osif_transport_add_io_handler, ret %d", (FMT__D, ret));

  return ret;
}


zb_ret_t osif_transport_remove_io_handler(osif_ipc_handle_t handle)
{
  zb_ret_t ret;
  zb_uindex_t handler_index;

  TRACE_MSG(TRACE_OSIF2, ">> osif_transport_remove_io_handler, handle %d", (FMT__D, handle));

  ret = find_handler_record_by_handle(handle, &handler_index);

  if (ret == RET_OK)
  {
    free_handler_record(handler_index);
  }

  TRACE_MSG(TRACE_OSIF2, "<< osif_transport_remove_io_handler, ret %d", (FMT__D, ret));

  return ret;
}


void osif_transport_init(void)
{
  ZB_BZERO(&transport_ctx, sizeof(transport_ctx));
}
