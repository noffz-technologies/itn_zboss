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
/* PURPOSE: Header file for Light Coordinator
*/

#ifndef LIGHT_ZC_H
#define LIGHT_ZC_H 1

#include "zboss_api.h"

/* Default channel */
#define LIGHT_ZC_CHANNEL_MASK (1l<<16)
/* IEEE address of ZC */
#define LIGHT_ZC_ADDRESS {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa}

/* Enable joined device check */
#define ZC_AUTO_SEARCH_AND_BIND
#define ZC_AUTO_SEARCH_AND_BIND_LVL_CTRL_CLST

#ifdef ZC_AUTO_SEARCH_AND_BIND

/* Joined device types */
enum simple_dev_type_e
{
  SIMPLE_DEV_TYPE_UNUSED,
  SIMPLE_DEV_TYPE_UNDEFINED,
  SIMPLE_DEV_TYPE_LIGHT,
  SIMPLE_DEV_TYPE_LIGHT_CONTROL,
};

/* Level control roles enumeration */
enum simple_dev_match_step_e
{
  MATCH_STEP_ON_OFF_LVL_CTRL_SERVER,
  MATCH_STEP_ON_OFF_LVL_CTRL_CLIENT,
};

/* Binding steps enumeration */
enum simple_dev_bind_step_e
{
  BIND_STEP_ON_OFF_CLST,
#ifdef ZC_AUTO_SEARCH_AND_BIND_LVL_CTRL_CLST
  BIND_STEP_LVL_CTRL_CLST,
#endif
};

/* Maximum number of devices that can be checked */
#define LIGHT_ZC_MAX_DEVICES 2

/* Context to store joined devices */
typedef struct simple_device_s
{
  zb_uint8_t     dev_type;
  zb_uint8_t     match_step;
  zb_uint8_t     bind_step;
  zb_uint8_t     assign_idx;
  zb_uint8_t     last_zdo_tsn;
  zb_uint8_t     match_ep;
  zb_uint8_t     assign_table[LIGHT_ZC_MAX_DEVICES - 1];
  zb_uint16_t    short_addr;
  zb_ieee_addr_t ieee_addr;
} simple_device_t;

/* Global device context */
typedef struct light_zc_ctx_s
{
  simple_device_t devices[LIGHT_ZC_MAX_DEVICES];
} light_zc_ctx_t;

#endif  /* ZC_AUTO_SEARCH_AND_BIND */

#endif /* LIGHT_H */
