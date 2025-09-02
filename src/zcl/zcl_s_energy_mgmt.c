/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
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
/* PURPOSE: SERVER: Energy Management cluster implementation.
*/

#define ZB_TRACE_FILE_ID 73

#include "zb_common.h"

#if defined (ZB_ZCL_SUPPORT_CLUSTER_ENERGY_MANAGEMENT) || defined DOXYGEN

#include "zboss_api.h"
#include "zcl/zb_zcl_energy_mgmt.h"

zb_uint8_t gs_energy_management_server_received_commands[] =
{
  ZB_ZCL_CLUSTER_ID_ENERGY_MANAGEMENT_SERVER_ROLE_RECEIVED_CMD_LIST
};

zb_uint8_t gs_energy_management_server_generated_commands[] =
{
  ZB_ZCL_CLUSTER_ID_ENERGY_MANAGEMENT_SERVER_ROLE_GENERATED_CMD_LIST
};

zb_discover_cmd_list_t gs_energy_management_server_cmd_list =
{
  sizeof(gs_energy_management_server_received_commands), gs_energy_management_server_received_commands,
  sizeof(gs_energy_management_server_generated_commands), gs_energy_management_server_generated_commands
};

static void zb_zcl_energy_management_server_handle_manage_event(zb_uint8_t param, const zb_zcl_parsed_hdr_t *cmd_info)
{
  zb_zcl_energy_management_manage_event_payload_t pl_in;
  zb_zcl_energy_management_report_event_status_payload_t pl_out;
  zb_uint8_t *data_ptr = zb_buf_begin(param);
  zb_addr_u dst_addr = { .addr_short = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr };

  TRACE_MSG(TRACE_ZCL1, ">> zb_zcl_energy_management_server_handle_manage_event", (FMT__0));

  if (!ZB_ZCL_ENERGY_MANAGEMENT_MANAGE_EVENT_PAYLOAD_SIZE_IS_VALID(zb_buf_len(param)))
  {
    TRACE_MSG(TRACE_ZCL1, "Invalid packet len (%hd).", (FMT__H, zb_buf_len(param)));
    zb_zcl_send_default_handler(param, cmd_info, ZB_ZCL_STATUS_INVALID_FIELD);
    return;
  }

  ZB_ZCL_PACKET_GET_DATA32(&pl_in.issuer_event_id, data_ptr);
  ZB_ZCL_PACKET_GET_DATA16(&pl_in.device_class, data_ptr);
  ZB_ZCL_PACKET_GET_DATA8(&pl_in.utility_enrollment_group, data_ptr);
  ZB_ZCL_PACKET_GET_DATA8(&pl_in.actions_required, data_ptr);

  ZB_ZCL_DEVICE_CMD_PARAM_INIT_WITH(param, ZB_ZCL_ENERGY_MANAGEMENT_MANAGE_EVENT_CB_ID,
                                    RET_ERROR, cmd_info, &pl_in, &pl_out);

  if (ZCL_CTX().device_cb)
  {
    ZCL_CTX().device_cb(param);
  }

  if (RET_OK == ZB_ZCL_DEVICE_CMD_PARAM_STATUS(param))
  {
    zb_zcl_energy_management_server_send_report_event_status(param, &dst_addr,
                                                             ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                                             ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
                                                             ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
                                                             &pl_out,
                                                             NULL);
  }
  else
  {
    TRACE_MSG(TRACE_ZCL1, "<< error in user cb call:%d", (FMT__D, ZB_ZCL_DEVICE_CMD_PARAM_STATUS(param)));
    return;
  }
  TRACE_MSG(TRACE_ZCL1, "<< zb_zcl_energy_management_server_handle_manage_event", (FMT__0));
}


static zb_bool_t zb_zcl_process_energy_management_server_commands(zb_uint8_t param, const zb_zcl_parsed_hdr_t *cmd_info)
{
  zb_bool_t processed = ZB_FALSE;

  switch ((zb_zcl_energy_management_cli_cmd_t) cmd_info->cmd_id)
  {
    case ZB_ZCL_ENERGY_MANAGEMENT_CLI_CMD_MANAGE_EVENT:
      zb_zcl_energy_management_server_handle_manage_event(param, cmd_info);
      processed = ZB_TRUE;
      break;
    default:
      break;
  }

  /* Allow ZCL to handle status of command performing: decide to send default
  response or not, release buffer and etc. */

  return processed;
}


void zb_zcl_energy_management_server_send_report_event_status(zb_uint8_t param, zb_addr_u *dst_addr,
                                                             zb_aps_addr_mode_t dst_addr_mode, zb_uint8_t dst_ep,
                                                             zb_uint8_t src_ep,
                                                             zb_zcl_energy_management_report_event_status_payload_t *payload,
                                                             zb_callback_t cb)
{
  zb_zcl_energy_management_report_event_status_payload_t pl;
  zb_uint8_t *data = (zb_uint8_t *)&pl;

  TRACE_MSG(TRACE_ZCL1, ">> zb_zcl_energy_management_server_send_report_event_status", (FMT__0));

  ZB_BZERO(&pl, sizeof(pl));

  ZB_ZCL_PACKET_PUT_DATA32(data, &payload->issuer_event_id);
  ZB_ZCL_PACKET_PUT_DATA8(data, payload->event_status);
  ZB_ZCL_PACKET_PUT_DATA32(data, &payload->event_status_time);
  ZB_ZCL_PACKET_PUT_DATA8(data, payload->criticality_level_applied);
  ZB_ZCL_PACKET_PUT_DATA16(data, &payload->cooling_temperature_set_point_applied);
  ZB_ZCL_PACKET_PUT_DATA16(data, &payload->heating_temperature_set_point_applied);
  ZB_ZCL_PACKET_PUT_DATA8(data, payload->average_load_adjustment_percentage_applied);
  ZB_ZCL_PACKET_PUT_DATA8(data, payload->duty_cycle_applied);
  ZB_ZCL_PACKET_PUT_DATA8(data, payload->event_control);

  zb_zcl_send_cmd(param,
    dst_addr, dst_addr_mode, dst_ep,
    ZB_ZCL_FRAME_DIRECTION_TO_CLI,
    src_ep,
    &pl, sizeof(pl), NULL,
    ZB_ZCL_CLUSTER_ID_ENERGY_MANAGEMENT,
    ZB_ZCL_ENABLE_DEFAULT_RESPONSE,
    ZB_ZCL_ENERGY_MANAGEMENT_SRV_CMD_REPORT_EVENT_STATUS,
    cb
  );
  TRACE_MSG(TRACE_ZCL1, "<< zb_zcl_energy_management_server_send_report_event_status", (FMT__0));
}


zb_bool_t zb_zcl_process_s_energy_management_specific_commands(zb_uint8_t param)
{
  zb_zcl_parsed_hdr_t cmd_info;

  TRACE_MSG(TRACE_ZCL1, ">> zb_zcl_process_s_energy_management_specific_commands", (FMT__0));

  if ( ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param )
  {
    ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_energy_management_server_cmd_list;
    return ZB_TRUE;
  }

  ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

  /* ZB_ASSERT(cmd_info.profile_id == ZB_AF_SE_PROFILE_ID); */
  ZB_ASSERT(cmd_info.cluster_id == ZB_ZCL_CLUSTER_ID_ENERGY_MANAGEMENT);

  if(ZB_ZCL_FRAME_DIRECTION_TO_SRV == cmd_info.cmd_direction)
  {
    return zb_zcl_process_energy_management_server_commands(param, &cmd_info);
  }
  TRACE_MSG(TRACE_ZCL1, "<< zb_zcl_process_s_energy_management_specific_commands", (FMT__0));
  return ZB_FALSE;
}


static zb_ret_t check_value_energy_management(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t *value);

void zb_zcl_energy_management_init_server()
{
  zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_ENERGY_MANAGEMENT,
                              ZB_ZCL_CLUSTER_SERVER_ROLE,
                              (zb_zcl_cluster_check_value_t)check_value_energy_management,
                              (zb_zcl_cluster_write_attr_hook_t)NULL,
                              zb_zcl_process_s_energy_management_specific_commands);
}


static zb_ret_t check_value_energy_management(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t *value)
{
  ZVUNUSED(attr_id);
  ZVUNUSED(value);
  ZVUNUSED(endpoint);

  /* All values for mandatory attributes are allowed, extra check for
   * optional attributes is needed */

  return RET_OK;
}

#endif /* ZB_ZCL_SUPPORT_CLUSTER_ENERGY_MANAGEMENT || defined DOXYGEN */
