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

#define ZB_TRACE_FILE_ID 40142
#include "light_control.h"
#include "light_control_hal.h"
#include "zb_led_button.h"


#define BULB_LED_POWER 0
#define BULB_LED_CONNECT 1

#define BULB_BUTTON_1_IDX ZB_BOARD_BUTTON_0
ZB_ASSERT_COMPILE_DECL(BULB_BUTTON_1_IDX == LIGHT_CONTROL_BUTTON_ON);
#define BULB_BUTTON_2_IDX ZB_BOARD_BUTTON_1
ZB_ASSERT_COMPILE_DECL(BULB_BUTTON_2_IDX == LIGHT_CONTROL_BUTTON_OFF);

void light_control_send_on_off(zb_uint8_t param, zb_uint16_t on_off);

void button1_handler(zb_uint8_t param)
{
  ZVUNUSED(param);

#ifdef ZB_USE_BUTTONS
  light_control_button_pressed(LIGHT_CONTROL_BUTTON_ON);
#endif
}

void button2_handler(zb_uint8_t param)
{
  ZVUNUSED(param);

#ifdef ZB_USE_BUTTONS
  light_control_button_pressed(LIGHT_CONTROL_BUTTON_OFF);
#endif
}


/* Private functions */
void light_control_hal_device_started(void)
{
  zb_osif_led_on(BULB_LED_POWER);

}

void light_control_hal_gpio_init(void)
{

  zb_osif_led_button_init();

  if (zb_osif_button_state(BULB_BUTTON_1_IDX))
  {
    if (zb_osif_button_state(BULB_BUTTON_2_IDX))
    {
      zb_osif_led_off(BULB_LED_POWER);
    }
  }
#ifdef ZB_USE_BUTTONS
  zb_button_register_handler(BULB_BUTTON_1_IDX, 0, button1_handler);
  zb_button_register_handler(BULB_BUTTON_2_IDX, 0, button2_handler);
#endif
}

/* Public interface */
void light_control_hal_init(void)
{
  light_control_hal_gpio_init();
  light_control_hal_device_started();
}

zb_bool_t light_control_hal_is_button_pressed(zb_uint8_t button_no)
{
  zb_bool_t ret;

  ret = (zb_bool_t) zb_osif_button_state(button_no);

  return ret;
}

void light_control_hal_set_connect(zb_bool_t on)
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
