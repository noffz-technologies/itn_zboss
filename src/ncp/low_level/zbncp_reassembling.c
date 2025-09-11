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
/*  PURPOSE: NCP implementation of reassembling a packet from fragments.
*/

#define ZB_TRACE_FILE_ID 26

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_ll_pkt.h"
#include "zbncp.h"
#include "zbncp_frag_internal.h"


static zbncp_memref_t zbncp_res_cut_place_for_fragment(zbncp_res_ctx_t *res);
static zbncp_size_t zbncp_res_store_fragment(zbncp_res_ctx_t *res, const zbncp_ll_rx_info_t *info);


void zbncp_res_initialize(zbncp_res_ctx_t *res)
{
  res->user_buf = ZBNCP_NULL;
  res->max_size = 0;
  res->offset = 0;
  res->mem = zbncp_res_cut_place_for_fragment(res);
  zbncp_mem_fill(res->big_buf, ZBNCP_BIG_BUF_SIZE, 0);
}


void zbncp_res_set_place_for_rx_pkt(zbncp_res_ctx_t *res, void* buf, zbncp_size_t size)
{
  res->user_buf = buf;
  res->max_size = size;
}


zbncp_memref_t zbncp_res_get_place_for_fragment(const zbncp_res_ctx_t *res)
{
  return res->mem;
}


static zbncp_memref_t zbncp_res_cut_place_for_fragment(zbncp_res_ctx_t *res)
{
  zbncp_size_t fragment_size;

  if (res->offset + ZBNCP_LL_BODY_SIZE_MAX < ZBNCP_BIG_BUF_SIZE)
  {
    fragment_size = ZBNCP_LL_BODY_SIZE_MAX;
  }
  else
  {
    fragment_size = ZBNCP_BIG_BUF_SIZE - res->offset;
  }

  return zbncp_make_memref(res->big_buf + res->offset, fragment_size);
}


static zbncp_size_t zbncp_res_store_fragment(zbncp_res_ctx_t *res, const zbncp_ll_rx_info_t *info)
{
  zbncp_size_t rx_bytes = 0;

  res->offset += info->rxbytes;
  if ((info->flags & (ZBNCP_LL_PKT_END << ZBNCP_LL_PKT_LIM_SHIFT)) != 0u)
  {
    if (res->user_buf != ZBNCP_NULL)
    {
      rx_bytes = size_min(res->offset, res->max_size);
      zbncp_mem_copy(res->user_buf, res->big_buf, rx_bytes);
      zbncp_res_initialize(res);
    }
    else
    {
      res->mem = zbncp_make_memref(ZBNCP_NULL, 0);
    }
  }
  else
  {
    res->mem = zbncp_res_cut_place_for_fragment(res);
  }

  return rx_bytes;
}


zbncp_size_t zbncp_res_process_response(zbncp_res_ctx_t *res, const zbncp_ll_rx_info_t *info)
{
  zbncp_size_t rx_bytes = 0;

  if (info->rxbytes != 0u)
  {
    if (res->offset != 0u)
    {
      if ((info->flags & (ZBNCP_LL_PKT_START << ZBNCP_LL_PKT_LIM_SHIFT)) != 0u)
      {
        /* Reset the reassembling logic.
         * The first fragment was already received and the reassembly process was started.
         * It is expected that a packet with the END bit or no positioning bits will be received next.
         * If a packet with the START bit is received, it means that the other side aborted the
         * previous transmission and started a new one.
         * In such case, it is better to reset the logic and receive the next packet
         * than ingore frames and stay in this state forever.
         */
        zbncp_mem_move(res->big_buf, &res->big_buf[res->offset], info->rxbytes);
        res->offset = 0;
      }

      rx_bytes = zbncp_res_store_fragment(res, info);
    }
    else
    {
      /* waiting for the first fragment */
      if ((info->flags & (ZBNCP_LL_PKT_START << ZBNCP_LL_PKT_LIM_SHIFT)) != 0u)
      {
        rx_bytes = zbncp_res_store_fragment(res, info);
      }
      else
      {
        /* skip the fragment since we are waiting for the first fragment */
      }
    }
  }

  return rx_bytes;
}
