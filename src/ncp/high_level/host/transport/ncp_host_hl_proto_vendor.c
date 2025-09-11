/*
* Copyright (c) 2021, Nordic Semiconductor ASA
*
* All rights reserved.
*
* Commercial Usage
* Licensees holding valid Commercial licenses may use
* this file in accordance with the Commercial License
* Agreement provided with the Software or, alternatively, in accordance
* with the terms contained in a written agreement relevant to the usage of the file.
*/
/* PURPOSE: NCP High level transport implementation for the host side: VENDOR category
*/

#define ZB_TRACE_FILE_ID 113

#include "ncp_host_hl_proto.h"
#include "ncp/ncp_common_api.h"
#include "ncp/ncp_host_api.h"

#define NCP_HOST_VENDOR_RESP_CB_TAB_SIZE 5

typedef struct ncp_vendor_context_s
{
  struct response_cbs_s
  {
    zb_uint8_t tsn;
    zb_bool_t used;
    zb_ncp_custom_response_cb_t cb;
  } response_cbs [NCP_HOST_VENDOR_RESP_CB_TAB_SIZE];

  zb_ncp_custom_indication_cb_t indication_cb;
} ncp_vendor_context_t;

static ncp_vendor_context_t vendor_context;


static zb_ret_t get_cb_by_tsn(zb_uint8_t tsn, zb_ncp_custom_response_cb_t *cb)
{
  for (zb_uint8_t i=0; i<NCP_HOST_VENDOR_RESP_CB_TAB_SIZE; i++)
  {
    if ((vendor_context.response_cbs[i].tsn == tsn) && vendor_context.response_cbs[i].used)
    {
      *cb = vendor_context.response_cbs[i].cb;
      vendor_context.response_cbs[i].used = ZB_FALSE;
      return RET_OK;
    }
  }

  return RET_NOT_FOUND;
}


static void response_callback_exec(zb_uint8_t param, zb_uint16_t tsn)
{
  zb_ncp_custom_response_cb_t cb;

  if (get_cb_by_tsn((zb_uint8_t)tsn, &cb) == RET_OK)
  {
      cb(param);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Received response with unexpected TSN!", (FMT__0));
  }

  zb_buf_free(param);
}


static void indication_callback_exec(zb_uint8_t param)
{
  if (vendor_context.indication_cb != NULL)
  {
    vendor_context.indication_cb(param);
  }

  zb_buf_free(param);
}


static void handle_response(ncp_hl_response_header_t* response_header, zb_uint16_t len)
{
  zb_uint8_t buf = ZB_BUF_INVALID;
  ncp_hl_custom_resp_t *response_args;
  ncp_host_hl_rx_buf_handle_t body;

  ncp_host_hl_init_response_body(response_header, len, &body);

  buf = zb_buf_get(ZB_TRUE, body.len);
  if(buf == ZB_BUF_INVALID)
  {
    TRACE_MSG(TRACE_ERROR, "zb_buf_get failed!", (FMT__0));
    return;
  }

  zb_buf_initial_alloc(buf, body.len);
  response_args = ZB_BUF_GET_PARAM(buf, ncp_hl_custom_resp_t);

  response_args->tsn = response_header->tsn;
  response_args->status = ERROR_CODE(response_header->status_category, response_header->status_code);
  ncp_host_hl_buf_get_array(&body, (zb_uint8_t*)zb_buf_begin(buf), body.len);

  ZB_SCHEDULE_CALLBACK2(response_callback_exec, buf, response_header->tsn);
}


static void handle_indication(ncp_hl_ind_header_t* indication_header, zb_uint16_t len)
{
  zb_uint8_t buf = ZB_BUF_INVALID;
  ncp_host_hl_rx_buf_handle_t body;

  ncp_host_hl_init_indication_body(indication_header, len, &body);

  buf = zb_buf_get(ZB_TRUE, body.len);
  if (buf == ZB_BUF_INVALID)
  {
    TRACE_MSG(TRACE_ERROR, "zb_buf_get failed!", (FMT__0));
    return;
  }

  zb_buf_initial_alloc(buf, body.len);
  ncp_host_hl_buf_get_array(&body, (zb_uint8_t*)zb_buf_begin(buf), body.len);

  ZB_SCHEDULE_CALLBACK(indication_callback_exec, buf);
}


static zb_ret_t get_free_resp_cb_idx(zb_uint8_t *cb_idx)
{
  zb_uint8_t i;

  for (i = 0; i < NCP_HOST_VENDOR_RESP_CB_TAB_SIZE; i++)
  {
    if (!vendor_context.response_cbs[i].used)
    {
      *cb_idx = i;
      return RET_OK;
    }
  }

  return RET_TABLE_FULL;
}


static void register_response_cb(zb_uint8_t response_cbs_idx, zb_ncp_custom_response_cb_t cb, zb_uint8_t tsn)
{
    vendor_context.response_cbs[response_cbs_idx].used = ZB_TRUE;
    vendor_context.response_cbs[response_cbs_idx].cb = cb;
    vendor_context.response_cbs[response_cbs_idx].tsn = tsn;
}


zb_uint8_t ncp_host_custom_request(zb_uint8_t buf, zb_ncp_custom_response_cb_t cb)
{
  zb_uint8_t tsn = NCP_TSN_RESERVED;
  ncp_hl_custom_resp_t *bad_resp_args = ZB_BUF_GET_PARAM(buf, ncp_hl_custom_resp_t);
  zb_ret_t ret;
  zb_uint8_t resp_cb_idx;
  ncp_host_hl_tx_buf_handle_t body;


  ret = get_free_resp_cb_idx(&resp_cb_idx);
  if (ret != RET_OK)
  {
    bad_resp_args->status = ret;
    bad_resp_args->tsn = tsn;

    TRACE_MSG(TRACE_ERROR, "No free entry for new response callback", (FMT__0));

    cb(buf);
    zb_buf_free(buf);

    return tsn;
  }

  if (!ncp_host_get_buf_for_nonblocking_request(NCP_HL_VENDOR_SPECIFIC, &body, &tsn))
  {
    ncp_host_hl_buf_put_array(&body, (zb_uint8_t*)zb_buf_begin(buf), (zb_size_t)zb_buf_len(buf));
    ret = ncp_host_hl_send_packet(&body);
  }
  else
  {
    ret = RET_ERROR;
  }

  if (ret == RET_OK)
  {
    register_response_cb(resp_cb_idx, cb, tsn);
  }
  else
  {
    bad_resp_args->status = ret;
    bad_resp_args->tsn = tsn;
    cb(buf);
  }

  zb_buf_free(buf);
  return tsn;
}


void ncp_host_custom_register_ind_cb(zb_ncp_custom_indication_cb_t cb)
{
  vendor_context.indication_cb = cb;
}


void ncp_host_handle_vendor_response(void *data, zb_uint16_t len)
{
  ncp_hl_response_header_t *rh = (ncp_hl_response_header_t*)data;

  TRACE_MSG(TRACE_TRANSPORT3, ">> ncp_host_handle_vendor_response", (FMT__0));

  /* only NCP_HL_VENDOR_SPECIFIC in NCP_HL_CATEGORY_VENDOR req/resp */
  ZB_ASSERT(rh->hdr.call_id == NCP_HL_VENDOR_SPECIFIC);

  handle_response(rh, len);

  TRACE_MSG(TRACE_TRANSPORT3, "<< ncp_host_handle_vendor_response", (FMT__0));
}


void ncp_host_handle_vendor_indication(void *data, zb_uint16_t len)
{
  ncp_hl_ind_header_t* indication_header = (ncp_hl_ind_header_t*)data;

  TRACE_MSG(TRACE_TRANSPORT3, ">> ncp_host_handle_vendor_indication", (FMT__0));

  /* only NCP_HL_VENDOR_SPECIFIC_IND in NCP_HL_CATEGORY_VENDOR indications */
  ZB_ASSERT(indication_header->call_id == NCP_HL_VENDOR_SPECIFIC_IND);

  handle_indication(indication_header, len);

  TRACE_MSG(TRACE_TRANSPORT3, "<< ncp_host_handle_vendor_indication", (FMT__0));
}
