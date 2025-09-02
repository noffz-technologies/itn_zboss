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
/* PURPOSE: application signal common functions
*/

#define ZB_TRACE_FILE_ID            1211

#include "zb_common.h"
#include "zb_nwk.h"
#include "zb_secur.h"
#include "zb_zdo_globals.h"


zb_zdo_app_signal_type_t zb_get_app_signal(zb_uint8_t param,
                                           zb_zdo_app_signal_hdr_t **sg_p)
{
  zb_zdo_app_signal_type_t sig_type;

  if (zb_buf_len(param) == 0UL)
  {
    sig_type = ZB_ZDO_SIGNAL_DEFAULT_START;
    if (sg_p != NULL)
    {
      *sg_p = NULL;
    }
  }
  else if (zb_buf_len(param) < sizeof(zb_zdo_app_signal_hdr_t))
  {
    sig_type = ZB_ZDO_SIGNAL_ERROR;
    if (sg_p != NULL)
    {
      *sg_p = NULL;
    }
  }
  else
  {
    /* Suppress warning about alignment. Buffer begin is aligned here for sure. */
    void *p = zb_buf_begin(param);
    zb_zdo_app_signal_hdr_t *sg = (zb_zdo_app_signal_hdr_t *)p;
    if (sg_p != NULL)
    {
      *sg_p = sg;
    }
    sig_type = (zb_zdo_app_signal_type_t)sg->sig_type;
  }
  return sig_type;
}

void *zb_app_signal_pack(zb_uint8_t param, zb_uint32_t signal_code,
                         zb_int16_t status, zb_uint8_t data_size)
{
  return zb_app_signal_pack_with_detailed_status(param,
                                                 signal_code,
                                                 status == RET_OK ? RET_OK : RET_ERROR,
                                                 data_size);
}


void *zb_app_signal_pack_with_detailed_status(zb_uint8_t param, zb_uint32_t signal_code,
                                              zb_ret_t status, zb_uint8_t data_size)
{
  zb_uint8_t *body;

  ZB_ASSERT(param);
  if (signal_code == ZB_ZDO_SIGNAL_DEFAULT_START && data_size == 0U)
  {
    /* Default (compatible with previous API) event has no body */
    body = zb_buf_reuse(param);
    zb_buf_set_status(param, status);
    ZVUNUSED(body);
    /*
     * Don't use tracing in this function - it breaks sleepy ED if ZDO trace
     * subsystem is enabled.
     */
    /* TRACE_MSG(TRACE_ZDO1, "zb_app_signal_pack ZB_ZDO_SIGNAL_DEFAULT_START"
     *           " status %d", (FMT__D, zb_buf_get_status(param)));
     */
    return NULL;
  }
  else
  {
    /*
     * Initial alloc creates aligned pointer. Signal_code size is 4 bytes,
     * so signal data is aligned as well.
     */
    body = zb_buf_initial_alloc(param, sizeof(signal_code) + data_size);
    ZB_MEMCPY(body, &signal_code, sizeof(signal_code));
    zb_buf_set_status(param, status);
    /* TRACE_MSG(TRACE_ZDO1, "zb_app_signal_pack sig %ld size %d status %d",
     *           (FMT__L_D_D, signal_code, data_size,
     *           zb_buf_get_status(param)));
     */
    return (void*)(body + sizeof(signal_code));
  }
}
