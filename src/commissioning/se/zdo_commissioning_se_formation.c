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
/*  PURPOSE: SE specific commissioning formation
*/

#define ZB_TRACE_FILE_ID 102
#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_nwk_nib.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"
#include "zb_nvram.h"

#if defined ZB_SE_COMMISSIONING && defined ZB_FORMATION

static void se_commissioning_formation_channels_mask(zb_channel_list_t list)
{
  se_pref_channels_create_mask((zb_bool_t)(ZSE_CTXC().commissioning.formation_retries == 0), list);
}


static void se_start_formation(zb_uint8_t param)
{
  TRACE_MSG(TRACE_ZDO1, "SE start Formation %hd", (FMT__H, param));

  /* SE uses centralized security, so always ZC. */
  ZB_ASSERT(zb_get_device_type() == ZB_NWK_DEVICE_TYPE_COORDINATOR);
  ZSE_CTXC().commissioning.state = SE_STATE_FORMATION;
  ZSE_CTXC().commissioning.formation_retries = 0;
  ZSE_CTXC().commissioning.startup_control = ZSE_STARTUP_UNCOMMISSIONED;
  se_minimal_tc_init();
  ZB_SCHEDULE_CALLBACK(zdo_start_formation, param);
}


void se_formation_force_link(void)
{
  zdo_formation_force_link();

  FORMATION_SELECTOR().start_formation = se_start_formation;
  FORMATION_SELECTOR().get_formation_channels_mask = se_commissioning_formation_channels_mask;
}


#ifdef ZB_COORDINATOR_ROLE

void zb_se_set_network_coordinator_role(zb_uint32_t channel_mask)
{
  se_commissioning_force_link();
  se_formation_force_link();
  zb_set_network_coordinator_role_with_mode(channel_mask, ZB_COMMISSIONING_SE);
}

#endif /* ZB_COORDINATOR_ROLE */

#endif /* ZB_SE_COMMISSIONING && ZB_FORMATION */
