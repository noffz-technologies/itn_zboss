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
/* PURPOSE: Dimmable light sample HAL
*/
#define ZB_TRACE_FILE_ID 40144
#include "bulb.h"
#include "bulb_hal.h"

#define BULB_LED_POWER 0
#define BULB_LED_CONNECT 1
#if ZB_N_LEDS > 2
#define BULB_LED_LIGHT 2
#elif ZB_N_LEDS == 2
/* Reuse one of indicator LEDs for the bulb light */
#define BULB_LED_LIGHT BULB_LED_CONNECT
#else
/* Not all platforms define ZB_N_LEDS properly and by default the number of
 * LEDs is one. Do not print warning in this case (it can lead to a build
 * failure), assume that number of LEDs is sufficient. */
#define BULB_LED_LIGHT 2
/* #warning "At least 2 LEDs are needed for dimmable light sample" */
#endif
#define BULB_BUTTON_2 ZB_BOARD_BUTTON_1

/* Public interface */
void bulb_hal_init(void)
{
  zb_osif_led_button_init();

  zb_osif_led_level_init(BULB_LED_LIGHT);

  zb_osif_led_on(BULB_LED_POWER);
}

void bulb_hal_set_level(zb_uint8_t level)
{
  zb_osif_led_on_set_level(level);
}

void bulb_hal_set_on_off(zb_bool_t on)
{
  if (on)
  {
    zb_osif_led_on(BULB_LED_LIGHT);
  }
  else
  {
    zb_osif_led_on_set_level(0);
    zb_osif_led_off(BULB_LED_LIGHT);
  }
}

void bulb_hal_set_connect(zb_bool_t on)
{
  if (on)
  {
    zb_osif_led_on(BULB_LED_CONNECT);
  }
  else
  {
    zb_osif_led_off(BULB_LED_CONNECT);
  }
}

zb_bool_t bulb_hal_is_button_pressed(zb_uint8_t button_no)
{
  zb_bool_t ret = ZB_FALSE;
  ret = zb_osif_button_state(button_no);
  return ret;
}
