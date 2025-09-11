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
/* PURPOSE: time functions implementation for 8051 for the Common bank
*/


#define ZB_TRACE_FILE_ID 128
#include "zboss_api_core.h"

#include "zb_common.h"
#include "zb_time.h"

#ifndef ZB_ALIEN_TIMER

/*! \addtogroup ZB_BASE */
/*! @{ */


void zb_timer_start(zb_time_t timeout)
{
  zb_time_t t_cur = ZB_TIMER_GET();
  zb_time_t t = ZB_TIME_ADD(t_cur, timeout);
  zb_time_t timer_stop;

  if (ZB_TIMER_CTX().canstop)
  {
    /* ZB_TIMER_CTX can be accessed from Timer ISR in this case */
    ZB_OSIF_GLOBAL_LOCK();
  }

  /* Use separate variable to avoid false-positive MISRAC2012-Rule-13.5 violation */
  timer_stop = ZB_TIMER_CTX().timer_stop;
  if (!ZB_TIMER_CTX().started
      /* must wake earlier then scheduled before */
      || ZB_TIME_GE(timer_stop, t)
      /* already missed our stop time */
      || ZB_TIME_GE(t_cur, timer_stop)
    )
  {
    ZB_TIMER_CTX().timer_stop = t;
    ZB_TIMER_CTX().started = ZB_TRUE;
  }

  if (ZB_TIMER_CTX().canstop)
  {
    /* ZB_TIMER_CTX can be accessed from Timer ISR in this case */
    ZB_OSIF_GLOBAL_UNLOCK();
  }

  if (!ZB_U2B(ZB_CHECK_TIMER_IS_ON()))
  {
    /* timer is stopped - start it */
    ZB_START_HW_TIMER();
  }
/*
#ifndef ZB8051
  TRACE_MSG(TRACE_OSIF3, "t_cur %d tmo %d, timer_stop %d", (FMT__D_D_D, t_cur, (int)timeout, (int)ZB_TIMER_CTX().timer_stop));
#endif
*/
}

void zb_timer_enable_stop(void)
{
  ZB_TIMER_CTX().canstop = ZB_TRUE;
}

void zb_timer_disable_stop(void)
{
  ZB_TIMER_CTX().canstop = ZB_FALSE;
}

void zb_timer_stop()
{
  if (ZB_TIMER_CTX().canstop)
  {
    ZB_TIMER_CTX().started = ZB_FALSE;
    ZB_STOP_HW_TIMER();
  }
}

/*! @} */

#endif                          /* ZB_ALIEN_TIMER */


ZB_WEAK_PRE zb_time_t ZB_WEAK zb_timer_get(void)
{
#ifdef ZB_TOOL
  return osif_current_time_to_be();
#else
  return ZB_TIMER_CTX().timer;
#endif  /* ZB_TOOL */
}


/*
 platform-specific zb_timer_get_precise_time() (used only by application/sniffer/zboss_sniffer.c). Moved out of there.
 Temporary keep in commented out form.
 */
#ifdef KILL_WHEN_FINALLY_RESOLVED

void zb_timer_get_precise_time(zb_time_t *bi_num, zb_uint16_t *bi_reminder_us)
{
  zb_time_t bi;
  zb_time_t bi2;
  zb_time_t reminder;

  ZB_ASSERT(bi_num);
  ZB_ASSERT(bi_reminder_us);

  bi = ZB_TIMER_CTX().timer;
  reminder = zb_osif_get_timer_reminder();
  bi2 = ZB_TIMER_CTX().timer;

  /* If timer moved, refine reminder */
  if (bi != bi2)
  {
    reminder = zb_osif_get_timer_reminder();
  }

  *bi_num = bi2;
  *bi_reminder_us = reminder;
}

#endif
