/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
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
/* PURPOSE: Header file for Simple GW
*/

#ifndef SIMPLE_GW_H
#define SIMPLE_GW_H 1

#include "zboss_api.h"
#include "zb_led_button.h"

/* CONFIG SECTION */
#define IAS_CIE_ENABLED 1

#if defined IAS_CIE_ENABLED
#include "ias_cie_addon.h"
#endif

/* Used endpoint number */
#define SIMPLE_GW_ENDPOINT 1

#undef ZB_USE_BUTTONS
/* IAS zone IEEE address */
#define SIMPLE_GW_IEEE_ADDR {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa}
/* Default channel */
#define SIMPLE_GW_CHANNEL_MASK (1l<<16)

/* Device settings definitions */
#define SIMPLE_GW_INVALID_DEV_INDEX 0xff
#define SIMPLE_GW_DEV_NUMBER 20
#define SIMPLE_GW_TOGGLE_ITER_TIMEOUT (5*ZB_TIME_ONE_SECOND)
#define SIMPLE_GW_RANDOM_TIMEOUT_VAL (15)
#define SIMPLE_GW_COMMUNICATION_PROBLEMS_TIMEOUT (30) /* 300 sec */
#define SIMPLE_GW_TOGGLE_TIMEOUT (ZB_RANDOM_VALUE(SIMPLE_GW_RANDOM_TIMEOUT_VAL)) /* * ZB_TIME_ONE_SECOND */

/* Reporting settings */
#define SIMPLE_GW_REPORTING_MIN_INTERVAL 30
#define SIMPLE_GW_REPORTING_MAX_INTERVAL(dev_idx) ((90 * ((dev_idx) + 1)))

/* Device states enumeration */
enum simple_gw_device_state_e
{
  NO_DEVICE,
  MATCH_DESC_DISCOVERY,
  IEEE_ADDR_DISCOVERY,
  CONFIGURE_BINDING,
  CONFIGURE_REPORTING,
  COMPLETED,
  COMPLETED_NO_TOGGLE,
#if defined IAS_CIE_ENABLED
  WRITE_CIE_ADDR,
#endif
};

/* Joined devices information context */
typedef ZB_PACKED_PRE struct simple_gw_device_params_s
{
  zb_uint8_t dev_state;
  zb_uint8_t endpoint;
  zb_uint16_t short_addr;
  zb_ieee_addr_t ieee_addr;
  zb_uint8_t pending_toggle;
} simple_gw_device_params_t;

/* Global device context */
typedef ZB_PACKED_PRE struct simple_gw_device_ctx_s
{
  simple_gw_device_params_t devices[SIMPLE_GW_DEV_NUMBER];
} ZB_PACKED_STRUCT simple_gw_device_ctx_t;

/* Declare type for persisting into nvram context */
typedef simple_gw_device_ctx_t simple_gw_device_nvram_dataset_t;

/* Startup sequence routines */
zb_uint16_t simple_gw_associate_cb(zb_uint16_t short_addr);
void simple_gw_dev_annce_cb(zb_uint16_t short_addr);
void simple_gw_leave_indication(zb_ieee_addr_t dev_addr);

/* Devices discovery routines */
void find_onoff_device(zb_uint8_t param, zb_uint16_t dev_idx);
void find_onoff_device_delayed(zb_uint8_t param);
void find_onoff_device_cb(zb_uint8_t param);
void find_onoff_device_tmo(zb_uint8_t param);

void device_ieee_addr_req(zb_uint8_t param, zb_uint16_t dev_idx);
void device_ieee_addr_req_cb(zb_uint8_t param);

void bind_device(zb_uint8_t param, zb_uint16_t dev_idx);
void bind_device_cb(zb_uint8_t param);

void configure_reporting(zb_uint8_t param, zb_uint16_t dev_idx);
void configure_reporting_cb(zb_uint8_t param);

/* Devices management routines */
zb_uint8_t simple_gw_get_dev_index_by_state(zb_uint8_t dev_state);
zb_uint8_t simple_gw_get_dev_index_by_short_addr(zb_uint16_t short_addr);
zb_uint8_t simple_gw_get_dev_index_by_ieee_addr(zb_ieee_addr_t ieee_addr);

void send_toggle_req(zb_uint8_t param, zb_uint16_t dev_idx);
void send_toggle_req_delayed(zb_uint8_t dev_idx);
void simple_gw_remove_and_rejoin_device_delayed(zb_uint8_t idx);
void simple_gw_remove_device_delayed(zb_uint8_t idx);

/* Persisting data into NVRAM routines */
zb_uint16_t simple_gw_get_nvram_data_size();
void simple_gw_nvram_read_app_data(zb_uint8_t page, zb_uint32_t pos, zb_uint16_t payload_length);
zb_ret_t simple_gw_nvram_write_app_data(zb_uint8_t page, zb_uint32_t pos);

/* Handler for specific zcl commands */
zb_uint8_t zcl_specific_cluster_cmd_handler(zb_uint8_t param);

#endif /* SIMPLE_GW_H */
