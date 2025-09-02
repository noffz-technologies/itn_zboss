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
/* PURPOSE: SERVER: Events cluster implementation.
*/

#define ZB_TRACE_FILE_ID 69

#include "zb_common.h"

#if defined (ZB_ZCL_SUPPORT_CLUSTER_EVENTS) || defined DOXYGEN

#include "zboss_api.h"
#include "zcl/zb_zcl_events.h"

zb_uint8_t gs_events_server_received_commands[] =
{
  ZB_ZCL_CLUSTER_ID_EVENTS_SERVER_ROLE_RECEIVED_CMD_LIST
};

zb_uint8_t gs_events_server_generated_commands[] =
{
  ZB_ZCL_CLUSTER_ID_EVENTS_SERVER_ROLE_GENERATED_CMD_LIST
};

zb_discover_cmd_list_t gs_events_server_cmd_list =
{
  sizeof(gs_events_server_received_commands), gs_events_server_received_commands,
  sizeof(gs_events_server_generated_commands), gs_events_server_generated_commands
};

/* 2017/08/23 NK:MEDIUM Do not put the code under ifdef - may return ZB_FALSE indicating that
 * command is not processed. */
/** DD:Fixed
 * Comment: Discuss with NK that it doesn't make a sense to add follow code
 * under #ifdef block. This functions should return default values to indicate
 * that commands are not handled or processed.
 */

/* FUNCTIONS FOR SERVER */

/*Handle get event log request from client*/
zb_ret_t zb_zcl_events_server_handle_get_event_log(zb_uint8_t param, const zb_zcl_parsed_hdr_t *cmd_info)
{
  /*TODO: need to implement*/
  ZVUNUSED(param);
  ZVUNUSED(cmd_info);
  return RET_OK;
}


/*Handle clear event log request from client*/
zb_ret_t zb_zcl_events_server_handle_clear_event_log_request(zb_uint8_t param, const zb_zcl_parsed_hdr_t *cmd_info)
{
  /*TODO: need to implement*/
  ZVUNUSED(param);
  ZVUNUSED(cmd_info);
  return RET_OK;
}


void zb_zcl_events_server_send_publish_event(zb_uint8_t param, zb_addr_u *dst_addr,
                                                  zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep,
                                                  zb_uint8_t src_ep, zb_zcl_events_publish_event_payload_t *payload)
{
/*TODO: need to implement*/;
ZVUNUSED(param);
ZVUNUSED(dst_addr);
ZVUNUSED(dst_addr_mode);
ZVUNUSED(dst_ep);
ZVUNUSED(src_ep);
ZVUNUSED(payload);
}


void zb_zcl_events_server_send_publish_event_log(zb_uint8_t param, zb_addr_u *dst_addr,
                                                      zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep,
                                                      zb_uint8_t src_ep, zb_zcl_events_publish_event_log_payload_t *payload)
{
/*TODO: need to implement*/;
ZVUNUSED(param);
ZVUNUSED(dst_addr);
ZVUNUSED(dst_addr_mode);
ZVUNUSED(dst_ep);
ZVUNUSED(src_ep);
ZVUNUSED(payload);
}


void zb_zcl_events_server_send_clear_event_log_response(zb_uint8_t param, zb_addr_u *dst_addr,
                                                             zb_uint8_t dst_addr_mode, zb_uint8_t dst_ep,
                                                             zb_uint8_t src_ep, zb_uint8_t *payload) /*zb_uint8_t zb_zcl_events_ClearedEventsLogs;*/
{
/*TODO: need to implement*/;
ZVUNUSED(param);
ZVUNUSED(dst_addr);
ZVUNUSED(dst_addr_mode);
ZVUNUSED(dst_ep);
ZVUNUSED(src_ep);
ZVUNUSED(payload);
}


/** Function to process particular server commands
 * @param - pointer to buffer
 * @cmd_info - pointer to parsed zcl header
 * @return ZB_TRUE or ZB_FALSE
 */
static zb_bool_t zb_zcl_process_events_server_commands(zb_uint8_t param, const zb_zcl_parsed_hdr_t *cmd_info)
{
  zb_bool_t processed = ZB_FALSE;

  switch ((zb_zcl_events_cli_cmd_t)cmd_info->cmd_id)
  {
    case ZB_ZCL_EVENTS_CLI_CMD_GET_EVENT_LOG:
      /* 2017/08/23 NK:MEDIUM Uncomment but return that command is not processed. */
      /* DD:Fixed */
      zb_zcl_events_server_handle_get_event_log(param, cmd_info);
      break;
    case ZB_ZCL_EVENTS_CLI_CMD_CLEAR_EVENT_LOG_REQUEST:
      zb_zcl_events_server_handle_clear_event_log_request(param, cmd_info);
      break;
    default:
      break;
  }
  return processed;
}


/*Function to choose one of command sets: client commands or server commands*/
zb_bool_t zb_zcl_process_s_events_specific_commands(zb_uint8_t param)
{
  zb_zcl_parsed_hdr_t cmd_info;

  TRACE_MSG(TRACE_ZCL1, ">> zb_zcl_process_s_events_specific_commands", (FMT__0));

  if ( ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param )
  {
    ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_events_server_cmd_list;
    return ZB_TRUE;
  }

  ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

  ZB_ASSERT(ZB_ZCL_CLUSTER_ID_EVENTS == cmd_info.cluster_id);

  /* 2017/08/23 NK:MEDIUM Remove - we put this check in zcl_common for all clusters. */
  /* DD:Fixed */
  TRACE_MSG(TRACE_ZCL1, "<< zb_zcl_process_s_events_specific_commands", (FMT__0));

  if(ZB_ZCL_FRAME_DIRECTION_TO_SRV == cmd_info.cmd_direction)
  {
      return zb_zcl_process_events_server_commands(param, &cmd_info);
  }
  return ZB_FALSE;
}

void zb_zcl_events_init_server()
{
  zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_EVENTS,
                              ZB_ZCL_CLUSTER_SERVER_ROLE,
                              (zb_zcl_cluster_check_value_t)NULL,
                              (zb_zcl_cluster_write_attr_hook_t)NULL,
                              zb_zcl_process_s_events_specific_commands);
}

#endif /* ZB_ZCL_SUPPORT_CLUSTER_EVENTS || defined DOXYGEN */
