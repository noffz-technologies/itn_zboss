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
/*  PURPOSE: SE+BDB specific commissioning
*/

#define ZB_TRACE_FILE_ID 103


#include "zb_common.h"
#include "zb_bdb_internal.h"

#ifdef ZB_SE_BDB_MIXED

static void se_switch_to_bdb_commissioning(zb_bdb_commissioning_mode_mask_t step)
{
  bdb_force_link();

  ZB_TCPOL().update_trust_center_link_keys_required = ZB_TRUE;
  COMM_CTX().commissioning_type = ZB_COMMISSIONING_BDB;
  ZB_BDB().bdb_commissioning_status = ZB_BDB_STATUS_IN_PROGRESS;
  ZB_BDB().bdb_commissioning_step = step;

  /* Should we set ZB_BDB().bdb_application_signal here? */

  /* BDB formation is not initialized here. We don't need BDB formation logic for SE coordinator. */
}


void zb_se_set_bdb_mode_enabled(zb_bool_t enabled)
{
  ZSE_CTXC().commissioning.allow_bdb_in_se_mode = enabled;
  ZSE_CTXC().commissioning.switch_to_bdb_commissioning = se_switch_to_bdb_commissioning;

  /* Enable BDB-style keys update and verify if BDB in SE mode is allowed. */
  /* Note: setting this for a ZC means it will remove a joiner if it doesn't request
     a TCLK during ZB_TCPOL().trust_center_node_join_timeout. This could be undesirable behavior. */
  zb_aib_tcpol_set_update_trust_center_link_keys_required(enabled);
}

#endif /* ZB_SE_BDB_MIXED */
