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
/*  PURPOSE: NCP implementation of divider a packet to a fragments.
*/

#define ZB_TRACE_FILE_ID 27

#include "zbncp_types.h"
#include "zbncp_mem.h"
#include "zbncp_ll_pkt.h"
#include "zbncp.h"
#include "zbncp_frag_internal.h"


static zbncp_ll_tx_pkt_t zbncp_div_cut_piece(zbncp_div_ctx_t *div);


void zbncp_div_initialize(zbncp_div_ctx_t *div)
{
  div->offset = 0;
  div->pkt_size = 0;
  div->tx_pkt.mem = zbncp_make_cmemref(ZBNCP_NULL, 0);
  div->tx_pkt.flags = 0;
  zbncp_mem_fill(div->big_buf, ZBNCP_BIG_BUF_SIZE, 0);
}


zbncp_int32_t zbncp_div_store_tx_pkt(zbncp_div_ctx_t *div, const void* buf, zbncp_size_t size)
{
  zbncp_int32_t ret = 0;

  /*it's a normal situation when data for transmission is off */
  if ((buf != ZBNCP_NULL) && (size != 0u))
  {
    if (0u == div->pkt_size)
    {
      if (size <= ZBNCP_BIG_BUF_SIZE)
      {
        zbncp_mem_copy(div->big_buf, buf, size);
        div->pkt_size = size;
        div->tx_pkt = zbncp_div_cut_piece(div);
      }
      else
      {
        ret = ZBNCP_RET_ERROR;
      }
    }
    else
    {
      ret = ZBNCP_RET_BUSY;
    }
  }

  return ret;
}


zbncp_ll_tx_pkt_t zbncp_div_request_fragment(const zbncp_div_ctx_t *div)
{
  return div->tx_pkt;
}


static zbncp_ll_tx_pkt_t zbncp_div_cut_piece(zbncp_div_ctx_t *div)
{
  zbncp_cmemref_t cmem;
  zbncp_size_t fragment_size;
  zbncp_uint8_t flags = 0;

  if (div->pkt_size != 0u)
  {
    if ((div->offset + ZBNCP_LL_BODY_SIZE_MAX) < div->pkt_size)
    {
      /* Try to send _small_ packet first. It gives us a bit of economy when Host is waking up NCP by sending that packet.
         But do not create extra packets.
       */
      if (div->offset == 0u)
      {
        fragment_size = div->pkt_size % ZBNCP_LL_BODY_SIZE_MAX;
        if (fragment_size == 0u)
        {
          fragment_size = ZBNCP_LL_BODY_SIZE_MAX;
        }
      }
      else
      {
        fragment_size = ZBNCP_LL_BODY_SIZE_MAX;
      }
    }
    else
    {
      fragment_size = div->pkt_size - div->offset;
      flags |= ZBNCP_LL_PKT_END << ZBNCP_LL_PKT_LIM_SHIFT;
    }

    if (0u == div->offset)
    {
      flags |= ZBNCP_LL_PKT_START << ZBNCP_LL_PKT_LIM_SHIFT;
    }

    cmem = zbncp_make_cmemref(div->big_buf + div->offset, fragment_size);
    div->offset += fragment_size;
  }
  else
  {
    cmem = zbncp_make_cmemref(ZBNCP_NULL, 0);
  }

  div->tx_pkt.mem = cmem;
  div->tx_pkt.flags = flags;

  return div->tx_pkt;
}


zbncp_bool_t zbncp_div_process_response(zbncp_div_ctx_t *div, const zbncp_ll_quant_res_t *rsp)
{
  if (rsp->txbytes != 0u)
  {
    if (div->offset == div->pkt_size)
    {
      zbncp_div_initialize(div);
    }
    else
    {
      div->tx_pkt = zbncp_div_cut_piece(div);
    }
  }

  return (zbncp_bool_t )(div->pkt_size == 0u);
}

