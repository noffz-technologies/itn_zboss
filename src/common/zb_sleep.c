/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2021 DSR Corporation, Denver CO, USA.
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
/* PURPOSE: Sleep mode routines
*/

#define ZB_TRACE_FILE_ID 44

#include "zb_common.h"

#ifdef ZB_USE_SLEEP
#include "zb_secur.h"
#include "zb_nwk.h"
#include "zb_zdo_globals.h"

void zb_sleep_init()
{
  ZG->sleep.threshold = ZB_SCHED_SLEEP_THRESHOLD_MS;
  ZG->sleep.permit_sleep_signal = ZB_TRUE;
  ZG->sleep.last_timestamp = ZB_TIMER_GET();

  /* By default - keep the timer always active for routers. */
#ifdef ZB_ED_ROLE
  zb_timer_enable_stop();
#endif /* ZB_ED_ROLE */
}

zb_ret_t zb_sleep_set_threshold(zb_uint32_t threshold_ms)
{
  if (threshold_ms <= ZB_MAXIMUM_SLEEP_THRESHOLD_MS && threshold_ms >= ZB_SCHED_SLEEP_THRESHOLD_MS)
  {
    ZG->sleep.threshold = threshold_ms;
    return RET_OK;
  }
  else
  {
    return RET_ERROR;
  }
}

zb_uint32_t zb_get_sleep_threshold()
{
  return ZG->sleep.threshold;
}

zb_uint32_t zb_sleep_calc_sleep_tmo()
{
  zb_uint32_t sleep_tmo = 0;

  /* Can sleep if no immediate callbacks scheduled. */
  if (ZB_RING_BUFFER_IS_EMPTY(ZB_CB_Q))
  {
    /* If delayed callbacks queue is not empty, calculate sleep_tmo */
    if (!ZB_POOLED_LIST8_IS_EMPTY(ZG->sched.tm_queue))
    {
      zb_time_t t = ZB_TIMER_GET();
      zb_tm_q_ent_t *ent = ZB_POOLED_LIST8_GET_HEAD(ZG->sched.tm_buffer, ZG->sched.tm_queue, next)
        + ZG->sched.tm_buffer;
      sleep_tmo = ZB_TIME_SUBTRACT(ent->run_time, t);
#if 0
      /* TODO: some extra sleep/awake pairs on hardware will be debugged and reduced in the future */
      TRACE_MSG(TRACE_APP4, "zb_sleep_calc_sleep_tmo tmo %d func %p t %u run_time %u (my addr %p)", (FMT__D_P_D_D_P, sleep_tmo, ent->func, t, ent->run_time, &zb_sleep_calc_sleep_tmo));
#endif
    }
    else
    {
      sleep_tmo = ZB_SCHED_SLEEP_NO_TM_BI;
    }
  }

  return ZB_TIME_BEACON_INTERVAL_TO_MSEC(sleep_tmo);
}

void zb_sleep_can_sleep(zb_uint32_t sleep_tmo)
{
  zb_bufid_t sig_buf = zb_buf_get_out();

  if (sig_buf != 0U)
  {
    zb_zdo_signal_can_sleep_params_t *can_sleep_params;
    TRACE_MSG(TRACE_COMMON3, "zb_sleep_can_sleep param %hd", (FMT__H, sig_buf));
    can_sleep_params = (zb_zdo_signal_can_sleep_params_t *)zb_app_signal_pack(sig_buf,
      ZB_COMMON_SIGNAL_CAN_SLEEP, RET_OK, (zb_uint8_t)sizeof(zb_zdo_signal_can_sleep_params_t));
    can_sleep_params->sleep_tmo = sleep_tmo;
    ZB_SCHEDULE_CALLBACK(zboss_signal_handler, sig_buf);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "zb_sleep_can_sleep: dont have a buffer for sig!", (FMT__0));
  }
}

void zb_sleep_now(void)
{
  zb_bool_t timer_was_on = (zb_bool_t)ZB_CHECK_TIMER_IS_ON();
  zb_uint32_t timer_orig_tmo = ZB_TIMER_CTX().timer_stop - ZB_TIMER_CTX().timer;
  zb_uint32_t slept_time_ms;
  zb_uint32_t slept_time_corrected_ms;
  zb_uint32_t slept_time_bi;
  zb_uint32_t sleep_tmo = zb_sleep_calc_sleep_tmo();

  if (sleep_tmo > ZG->sleep.threshold)
  {
    zb_timer_stop();
    slept_time_ms = zb_osif_sleep(sleep_tmo);

    /* zzzzZZZZZzzzz */

    if (slept_time_ms == ZB_SLEEP_INVALID_VALUE)
    {
      slept_time_ms = 0;
    }
    else
    {
      /* slept_time is correct, permit sleep signal */
      ZG->sleep.permit_sleep_signal = ZB_TRUE;
    }

    slept_time_corrected_ms = slept_time_ms + ZB_TIMER_CTX().sleep_tmo_remainder_ms;
    slept_time_bi = ZB_MILLISECONDS_TO_BEACON_INTERVAL_FLOOR(slept_time_corrected_ms);
    ZB_TIMER_CTX().sleep_tmo_remainder_ms =
      slept_time_corrected_ms - ZB_TIME_BEACON_INTERVAL_TO_MSEC(slept_time_bi);

    /* the remainder should be less or equal to 1 BI, '<=' instead of '<' because
       ZB_TIME_BEACON_INTERVAL_TO_MSEC rounds the result down here.
       '+ 1' due to ZB_TIME_BEACON_INTERVAL_TO_MSEC rounding down to ms during
       sleep_tmo_remainder_ms calculation */
    ZB_ASSERT(ZB_TIMER_CTX().sleep_tmo_remainder_ms <= ZB_TIME_BEACON_INTERVAL_TO_MSEC(1) + 1U);

    if (timer_was_on)
    {
#ifndef ZB_NSNG
      /* Continue original timer, but divide time which we already slept. */
      /* In case of NSNG nsng_move_time() already perform correction, so this step is not needed */
      if (ZB_TIMER_CTX().canstop)
      {
        /* Move timer if we stopped the timer */
        ZB_TIMER_CTX().timer += slept_time_bi;
      }
#endif
      if (timer_orig_tmo > slept_time_bi)
      {
        zb_timer_start(timer_orig_tmo - slept_time_bi);
      }
    }
    zb_osif_wake_up();

    if (slept_time_ms != 0U)
    {
      TRACE_MSG(TRACE_COMMON3, "slept_time %ld ms", (FMT__L, slept_time_ms));
    }
  }
}

#endif /* ZB_USE_SLEEP */
