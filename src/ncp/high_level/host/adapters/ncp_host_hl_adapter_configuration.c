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
/*  PURPOSE: NCP High level transport (adapters layer) implementation for the host side
*/
#define ZB_TRACE_FILE_ID 17514

#include "ncp_host_hl_transport_internal_api.h"
#include "ncp_host_soc_state.h"
#include "ncp_hl_proto.h"


void zb_secur_set_tc_rejoin_enabled(zb_bool_t enable)
{
  zb_ret_t ret;
  TRACE_MSG(TRACE_ZDO2, "zb_secur_set_tc_rejoin_enabled, enable %d",
            (FMT__D, enable));

  ret = ncp_host_set_tc_policy(NCP_HL_TC_POLICY_TC_REJOIN_ENABLED, enable);
  ZB_ASSERT(ret == RET_OK);
}


void zb_secur_set_ignore_tc_rejoin(zb_bool_t enable)
{
  zb_ret_t ret;

  TRACE_MSG(TRACE_ZDO2, "zb_secur_set_ignore_tc_rejoin, enable %d",
            (FMT__D, enable));

  ret = ncp_host_set_tc_policy(NCP_HL_TC_POLICY_IGNORE_TC_REJOIN, enable);
  ZB_ASSERT(ret == RET_OK);
}


void zb_zdo_set_aps_unsecure_join(zb_bool_t insecure_join)
{
  zb_ret_t ret;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_set_aps_unsecure_join, insecure_join %d",
            (FMT__D, insecure_join));

  ret = ncp_host_set_tc_policy(NCP_HL_TC_POLICY_APS_INSECURE_JOIN, insecure_join);
  ZB_ASSERT(ret == RET_OK);
}


void zb_zdo_disable_network_mgmt_channel_update(zb_bool_t disable)
{
  zb_ret_t ret;

  TRACE_MSG(TRACE_ZDO2, "zb_zdo_disable_network_mgmt_channel_update, disable %d",
            (FMT__D, disable));

  ret = ncp_host_set_tc_policy(NCP_HL_TC_POLICY_DISABLE_NWK_MGMT_CHANNEL_UPDATE, disable);
  ZB_ASSERT(ret == RET_OK);
}


void zb_secur_set_unsecure_tc_rejoin_enabled(zb_bool_t enable)
{
  zb_ret_t ret;

  TRACE_MSG(TRACE_ZDO2, "zb_secur_set_unsecure_tc_rejoin_enabled, enable %d",
            (FMT__D, enable));

  ret = ncp_host_set_tc_policy(NCP_HL_TC_POLICY_UNSECURE_TC_REJOIN_ENABLED, enable);
  ZB_ASSERT(ret == RET_OK);
}
