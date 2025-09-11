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
/* PURPOSE: General purpose software watchdog for ZB apps
*/


#define ZB_TRACE_FILE_ID 50
#include "zb_common.h"

#ifdef USE_ZB_WATCHDOG

#include "zb_time.h"
#include "zb_scheduler.h"
#include "zb_error_indication.h" /* ZB_CRITICAL_ERROR_WATCHDOG */
#include "zb_watchdog.h"

void zb_add_watchdog(zb_uint8_t wd_number, zb_time_t timeout)
{
  if (wd_number < ZB_N_WATCHDOG)
  {
    ZG->watchdog[wd_number].timeout = timeout;
    ZG->watchdog[wd_number].last_kick = ZB_TIMER_GET();
    ZG->watchdog[wd_number].state = ZB_WATCHDOG_ENABLED;
  }
  ZB_SCHEDULE_ALARM_CANCEL(zb_watchdog_scheduler, 0);
  ZB_SCHEDULE_ALARM(zb_watchdog_scheduler, 0, ZB_WATCHDOG_SCHED_QUANT);
}


void zb_kick_watchdog(zb_uint8_t wd_number)
{
  if (wd_number < ZB_N_WATCHDOG
      && ZG->watchdog[wd_number].timeout != 0)
  {
    ZG->watchdog[wd_number].last_kick = ZB_TIMER_GET();
  }
}


void zb_stop_watchdog(zb_uint8_t wd_number)
{
  zb_add_watchdog(wd_number, 0);
}

void zb_watchdog_scheduler(zb_uint8_t param)
{
  zb_uint_t i;

  (void)param;

  for (i = 0 ; i < ZB_N_WATCHDOG ; ++i)
  {
    if (ZG->watchdog[i].state == ZB_WATCHDOG_ENABLED &&
        ZG->watchdog[i].timeout != 0)
    {
      zb_time_t cur_t = ZB_TIMER_GET();
      zb_time_t last_t = ZB_TIME_ADD(ZG->watchdog[i].last_kick, ZG->watchdog[i].timeout);

      if (ZB_TIME_GE(cur_t, last_t) && cur_t != last_t)
      {
        TRACE_MSG(TRACE_ERROR, "ERROR: Watchdog (id %hd) timeout", (FMT__H, i));

        ZB_ERROR_RAISE(ZB_ERROR_SEVERITY_FATAL,
                       ERROR_CODE(ERROR_CATEGORY_WATCHDOG, ZB_ERROR_WATCHDOG_TRIGGERED),
                       (void*)i);
      }
    }
  }

  ZB_SCHEDULE_ALARM(zb_watchdog_scheduler, 0, ZB_WATCHDOG_SCHED_QUANT);
}

void zb_enable_watchdog(zb_uint8_t wd_number)
{
  if (wd_number < ZB_N_WATCHDOG)
  {
    ZG->watchdog[wd_number].state = ZB_WATCHDOG_ENABLED;
    /* INIT wd timer */
    zb_kick_watchdog(wd_number);
  }
}

void zb_disable_watchdog(zb_uint8_t wd_number)
{
  if (wd_number < ZB_N_WATCHDOG)
  {
    ZG->watchdog[wd_number].state = ZB_WATCHDOG_DISABLED;
  }
}

#endif  /* USE_ZB_WATCHDOG */
