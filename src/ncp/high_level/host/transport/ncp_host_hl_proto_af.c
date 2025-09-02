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
/*  PURPOSE: NCP High level transport implementation for the host side: AF category
*/

#define ZB_TRACE_FILE_ID 17

#include "ncp_host_hl_proto.h"

static void handle_af_set_simple_descriptor_response(ncp_hl_response_header_t* response,
                                                     zb_uint16_t len)
{
  zb_ret_t error_code = ERROR_CODE(response->status_category, response->status_code);
  ZVUNUSED(len);

  TRACE_MSG(TRACE_TRANSPORT3, ">> handle_af_set_simple_descriptor_response, status_code %d",
            (FMT__D, error_code));

  ncp_host_mark_blocking_request_finished();

  ncp_host_handle_af_set_simple_descriptor_response(error_code);

  TRACE_MSG(TRACE_TRANSPORT3, "<< handle_af_set_simple_descriptor_response", (FMT__0));
}

static void handle_af_delete_endpoint_response(ncp_hl_response_header_t* response,
                                               zb_uint16_t len)
{
  zb_ret_t error_code = ERROR_CODE(response->status_category, response->status_code);
  ZVUNUSED(len);

  TRACE_MSG(TRACE_TRANSPORT3, ">> handle_af_delete_endpoint_response, status_code %d",
            (FMT__D, error_code));

  ncp_host_mark_blocking_request_finished();

  ncp_host_handle_af_delete_endpoint_response(error_code);

  TRACE_MSG(TRACE_TRANSPORT3, "<< handle_af_delete_endpoint_response", (FMT__0));
}

static void handle_af_set_node_descriptor_response(ncp_hl_response_header_t* response,
                                                   zb_uint16_t len)
{
  zb_ret_t error_code = ERROR_CODE(response->status_category, response->status_code);
  ZVUNUSED(len);

  TRACE_MSG(TRACE_TRANSPORT3, ">> handle_af_set_node_descriptor_response, status_code %d",
            (FMT__D, error_code));

  ncp_host_mark_blocking_request_finished();

  ncp_host_handle_af_set_node_descriptor_response(error_code);

  TRACE_MSG(TRACE_TRANSPORT3, "<< handle_af_set_node_descriptor_response", (FMT__0));
}

static void handle_af_set_power_descriptor_response(ncp_hl_response_header_t* response,
                                                   zb_uint16_t len)
{
  zb_ret_t error_code = ERROR_CODE(response->status_category, response->status_code);
  ZVUNUSED(len);

  TRACE_MSG(TRACE_TRANSPORT3, ">> handle_af_set_power_descriptor_response, status_code %d",
            (FMT__D, error_code));

  ncp_host_mark_blocking_request_finished();

  ncp_host_handle_af_set_power_descriptor_response(error_code);

  TRACE_MSG(TRACE_TRANSPORT3, "<< handle_af_set_power_descriptor_response", (FMT__0));
}

void ncp_host_handle_af_response(void* data, zb_uint16_t len)
{
  ncp_hl_response_header_t* response_header = (ncp_hl_response_header_t*)data;

  TRACE_MSG(TRACE_TRANSPORT3, ">> ncp_host_handle_af_response", (FMT__0));

  switch(response_header->hdr.call_id)
  {
    case NCP_HL_AF_SET_SIMPLE_DESC:
      handle_af_set_simple_descriptor_response(response_header, len);
      break;
    case NCP_HL_AF_DEL_SIMPLE_DESC:
      handle_af_delete_endpoint_response(response_header, len);
      break;
    case NCP_HL_AF_SET_NODE_DESC:
      handle_af_set_node_descriptor_response(response_header, len);
      break;
    case NCP_HL_AF_SET_POWER_DESC:
      handle_af_set_power_descriptor_response(response_header, len);
      break;
    default:
      TRACE_MSG(TRACE_ERROR, "ncp_host_handle_af_response: unknown command ID 0x%hx",
                (FMT__H, response_header->hdr.call_id));
      ZB_ASSERT(0);
    }

   TRACE_MSG(TRACE_TRANSPORT3, "<< ncp_host_handle_af_response", (FMT__0));
}

void ncp_host_handle_af_indication(void* data, zb_uint16_t len)
{
  ncp_hl_response_header_t* response_header = (ncp_hl_response_header_t*)data;

  ZVUNUSED(len);

  TRACE_MSG(TRACE_ERROR, "ncp_host_handle_af_indication: unknown command ID 0x%hx",
            (FMT__H, response_header->hdr.call_id));

  ZB_ASSERT(0);
}

zb_ret_t ncp_host_af_set_simple_descriptor(zb_uint8_t  endpoint, zb_uint16_t app_profile_id,
                                           zb_uint16_t app_device_id, zb_bitfield_t app_device_version,
                                           zb_uint8_t app_input_cluster_count,
                                           zb_uint8_t app_output_cluster_count,
                                           zb_uint16_t *app_input_cluster_list,
                                           zb_uint16_t *app_output_cluster_list)
{
  zb_ret_t ret = RET_BUSY;
  ncp_host_hl_tx_buf_handle_t body;
  zb_uint8_t i;

  TRACE_MSG(TRACE_TRANSPORT2, ">> ncp_host_af_set_simple_descriptor", (FMT__0));

  if (!ncp_host_get_buf_for_blocking_request(NCP_HL_AF_SET_SIMPLE_DESC, &body, NULL))
  {
    ncp_host_hl_buf_put_u8(&body, endpoint);
    ncp_host_hl_buf_put_u16(&body, app_profile_id);
    ncp_host_hl_buf_put_u16(&body, app_device_id);
    ncp_host_hl_buf_put_u8(&body, app_device_version);
    ncp_host_hl_buf_put_u8(&body, app_input_cluster_count);
    ncp_host_hl_buf_put_u8(&body, app_output_cluster_count);

    for (i = 0; i < app_input_cluster_count; i++)
    {
      ncp_host_hl_buf_put_u16(&body, app_input_cluster_list[i]);
    }

    for (i = 0; i < app_output_cluster_count; i++)
    {
      ncp_host_hl_buf_put_u16(&body, app_output_cluster_list[i]);
    }

    ret = ncp_host_hl_send_packet(&body);
  }

  TRACE_MSG(TRACE_TRANSPORT2, "<< ncp_host_af_set_simple_descriptor, ret %d", (FMT__D, ret));

  return ret;
}

zb_ret_t ncp_host_af_delete_endpoint(zb_uint8_t endpoint)
{
  zb_ret_t ret = RET_BUSY;
  ncp_host_hl_tx_buf_handle_t body;

  TRACE_MSG(TRACE_TRANSPORT2, ">> ncp_host_af_delete_endpoint", (FMT__0));

  if (!ncp_host_get_buf_for_blocking_request(NCP_HL_AF_DEL_SIMPLE_DESC, &body, NULL))
  {
    ncp_host_hl_buf_put_u8(&body, endpoint);
    ret = ncp_host_hl_send_packet(&body);
  }

  TRACE_MSG(TRACE_TRANSPORT2, "<< ncp_host_af_delete_endpoint, ret %d", (FMT__D, ret));

  return ret;
}

zb_ret_t ncp_host_af_set_node_descriptor(zb_uint8_t device_type, zb_uint8_t mac_capabilities,
                                         zb_uint16_t manufacturer_code)
{
  zb_ret_t ret = RET_BUSY;
  ncp_host_hl_tx_buf_handle_t body;

  TRACE_MSG(TRACE_TRANSPORT2, ">> ncp_host_af_set_node_descriptor", (FMT__0));

  if (!ncp_host_get_buf_for_blocking_request(NCP_HL_AF_SET_NODE_DESC, &body, NULL))
  {
    ncp_host_hl_buf_put_u8(&body, device_type);
    ncp_host_hl_buf_put_u8(&body, mac_capabilities);
    ncp_host_hl_buf_put_u16(&body, manufacturer_code);
    ret = ncp_host_hl_send_packet(&body);
  }

  TRACE_MSG(TRACE_TRANSPORT2, "<< ncp_host_af_set_node_descriptor, ret %d", (FMT__D, ret));

  return ret;
}

zb_ret_t ncp_host_af_set_power_descriptor(zb_uint8_t current_power_mode,
                                          zb_uint8_t available_power_sources,
                                          zb_uint8_t current_power_source,
                                          zb_uint8_t current_power_source_level)
{
  zb_ret_t ret = RET_BUSY;
  ncp_host_hl_tx_buf_handle_t body;

  TRACE_MSG(TRACE_TRANSPORT2, ">> ncp_host_af_set_power_descriptor", (FMT__0));

  if (!ncp_host_get_buf_for_blocking_request(NCP_HL_AF_SET_POWER_DESC, &body, NULL))
  {
    ncp_host_hl_buf_put_u8(&body, current_power_mode);
    ncp_host_hl_buf_put_u8(&body, available_power_sources);
    ncp_host_hl_buf_put_u8(&body, current_power_source);
    ncp_host_hl_buf_put_u8(&body, current_power_source_level);
    ret = ncp_host_hl_send_packet(&body);
  }

  TRACE_MSG(TRACE_TRANSPORT2, "<< ncp_host_af_set_power_descriptor, ret %d", (FMT__D, ret));

  return ret;
}
