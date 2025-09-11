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
/* PURPOSE: Linux stubs for led/buttons support.
*/

#define ZB_TRACE_FILE_ID 32364
#include "zboss_api_core.h"
#include "zb_led_button.h"

#if 1 /*#ifdef ZB_USE_BUTTONS*/

#ifdef ZB_DEPRECATED_API
/* This function is not used anywhere and seems its objective is not different from
 * 'zb_osif_led_button_init()' which is currently used. It will be removed in a future
 * release. */
void zb_led_init(void)
{
  return;
}
#endif /* ZB_DEPRECATED_API */

void zb_osif_led_button_init(void)
{
  return;
}

void zb_osif_led_on(zb_uint8_t led_no)
{
  ZVUNUSED(led_no);
}

void zb_osif_led_off(zb_uint8_t led_no)
{
  ZVUNUSED(led_no);
}

void zb_led_blink_on(zb_uint8_t led_arg)
{
  ZVUNUSED(led_arg);
}

void zb_led_blink_off(zb_uint8_t led_arg)
{
  ZVUNUSED(led_arg);
}

void zb_osif_button_cb(zb_uint8_t arg)
{
  ZVUNUSED(arg);
}

zb_bool_t zb_osif_button_state(zb_uint8_t arg)
{
  /* Button is always switched off */
  ZVUNUSED(arg);
  return ZB_FALSE;
}

/* FIXME: That function defines in zb_gpmac_scheduler.c and use most probably only in LCGW.
   Need to resolve those dependencies in GP MAC and LCGW in a proper way.
*/
#ifndef ZB_GP_MAC
zb_bool_t zb_setup_buttons_cb(zb_callback_t cb)
{
  ZVUNUSED(cb);
  return ZB_TRUE;
}
#endif /* ZB_GP_MAC */


#endif  /* ZB_USE_BUTTONS */
