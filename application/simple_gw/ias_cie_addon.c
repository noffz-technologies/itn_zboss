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
/*  PURPOSE: Simple GW application IAS CIE additions
*/

#define ZB_TRACE_FILE_ID 63593

#include "simple_gw.h"
#include "simple_gw_device.h"

#if defined IAS_CIE_ENABLED

extern simple_gw_device_ctx_t g_device_ctx;
extern zb_ieee_addr_t g_zc_addr;

void simple_gw_remove_device_delayed(zb_uint8_t idx);

void find_ias_zone_device_tmo(zb_uint8_t dev_idx)
{
  TRACE_MSG(TRACE_APP1, "find_ias_zone_device_tmo", (FMT__0));

  ZB_ASSERT(dev_idx < SIMPLE_GW_DEV_NUMBER);
  if ((g_device_ctx.devices[dev_idx].dev_state != NO_DEVICE) &&
      (g_device_ctx.devices[dev_idx].dev_state != COMPLETED_NO_TOGGLE))
  {
    ZB_SCHEDULE_APP_CALLBACK(simple_gw_remove_device_delayed, dev_idx);
  }
}

void find_ias_zone_device_delayed(zb_uint8_t idx)
{
  zb_buf_get_out_delayed_ext(find_ias_zone_device, g_device_ctx.devices[idx].short_addr, 0);
}

void find_ias_zone_device(zb_uint8_t param, zb_uint16_t short_addr)
{
  zb_bufid_t buf = param;
  zb_zdo_match_desc_param_t *req;

  TRACE_MSG(TRACE_APP1, ">> find_ias_zone_device %hd", (FMT__H, param));

  req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_match_desc_param_t) + (1) * sizeof(zb_uint16_t));

  req->nwk_addr = short_addr;
  req->addr_of_interest = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
  req->profile_id = ZB_AF_HA_PROFILE_ID;
  /* We are searching for IAS ZONE Server */
  req->num_in_clusters = 1;
  req->num_out_clusters = 0;
  req->cluster_list[0] = ZB_ZCL_CLUSTER_ID_IAS_ZONE;

  zb_zdo_match_desc_req(param, find_ias_zone_device_cb);

  TRACE_MSG(TRACE_APP1, "<< find_ias_zone_device %hd", (FMT__H, param));
}

void find_ias_zone_device_cb(zb_uint8_t param)
{
  zb_bufid_t buf = param;
  zb_zdo_match_desc_resp_t *resp = (zb_zdo_match_desc_resp_t*)zb_buf_begin(buf);
  zb_uint8_t *match_ep = NULL;
  zb_apsde_data_indication_t *ind = NULL;
  zb_uint8_t dev_idx = SIMPLE_GW_INVALID_DEV_INDEX;

  TRACE_MSG(TRACE_APP1, ">> find_ias_zone_device_cb param %hd, resp match_len %hd", (FMT__H_H, param, resp->match_len));

  if (resp->status == ZB_ZDP_STATUS_SUCCESS)
  {
    ind = ZB_BUF_GET_PARAM(buf, zb_apsde_data_indication_t);
    dev_idx = simple_gw_get_dev_index_by_short_addr(ind->src_addr);

    if (dev_idx != SIMPLE_GW_INVALID_DEV_INDEX)
    {
      if (resp->match_len)
      {
      /* Match EP list follows right after response header */
      match_ep = (zb_uint8_t*)(resp + 1);

      /* we are searching for exact cluster, so only 1 EP maybe found */
      g_device_ctx.devices[dev_idx].short_addr = ind->src_addr;
      g_device_ctx.devices[dev_idx].endpoint = *match_ep;
      g_device_ctx.devices[dev_idx].dev_state = WRITE_CIE_ADDR;

      ZB_SCHEDULE_APP_ALARM_CANCEL(find_ias_zone_device_tmo, dev_idx);

      TRACE_MSG(TRACE_APP2, "found dev addr %d ep %hd dev_idx %hd",
                (FMT__D_H_H, g_device_ctx.devices[dev_idx].short_addr, g_device_ctx.devices[dev_idx].endpoint, dev_idx));

      /* Next step is to send CIE address */
      ZB_SCHEDULE_APP_CALLBACK2(write_cie_addr, param, dev_idx);
        param = 0;
    }
    else
    {
      g_device_ctx.devices[dev_idx].short_addr = ind->src_addr;
      /* It is neither onoff nor ias zone device, but lets keep it w/o any
      additional configuration. */
      g_device_ctx.devices[dev_idx].dev_state = COMPLETED_NO_TOGGLE;
      ZB_SCHEDULE_APP_ALARM_CANCEL(find_ias_zone_device_tmo, dev_idx);
    }
    }
    else
    {
      TRACE_MSG(TRACE_APP2, "Device not found!", (FMT__0));
    }
  }

  if (param)
  {
    zb_buf_free(param);
  }

  TRACE_MSG(TRACE_APP1, "<< find_ias_zone_device_cb", (FMT__0));
}

void write_cie_addr(zb_uint8_t param, zb_uint16_t dev_idx)
{
  zb_uint8_t *cmd_ptr;

  g_device_ctx.devices[dev_idx].dev_state = COMPLETED_NO_TOGGLE;

  ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ(param, cmd_ptr, ZB_ZCL_ENABLE_DEFAULT_RESPONSE);

  ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(cmd_ptr, ZB_ZCL_ATTR_IAS_ZONE_IAS_CIE_ADDRESS_ID,
                                          ZB_ZCL_ATTR_TYPE_IEEE_ADDR, (zb_uint8_t*)g_zc_addr);

  ZB_ZCL_GENERAL_SEND_WRITE_ATTR_REQ(param, cmd_ptr, g_device_ctx.devices[dev_idx].short_addr,
                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                     g_device_ctx.devices[dev_idx].endpoint, SIMPLE_GW_ENDPOINT,
                                     ZB_AF_HA_PROFILE_ID, ZB_ZCL_CLUSTER_ID_IAS_ZONE, NULL);
}

#endif  /* IAS_CIE_ENABLED */


