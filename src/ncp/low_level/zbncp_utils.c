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
/*  PURPOSE: NCP utility functions implementation.
*/

#define ZB_TRACE_FILE_ID 33
#include "zbncp_utils.h"
#include "zbncp_mem.h"
#include "zbncp_debug.h"

/*
 * CRC polynomials and algorithms are selected to provide
 * compatibility with existing ZBOSS macsplit protocol
 * (e.g. in the CRC-16 implementation an inital and output
 * values are not inverted).
 * This implementation is not optimal.
 */

#define ZBNCP_CRC8_GEN_POLY   0xb2u
#define ZBNCP_CRC16_GEN_POLY  0x8408u

#define ZBNCP_LOWEST_BIT(x)   ((x) & 1u)

zbncp_uint8_t zbncp_crc8(zbncp_uint8_t init, const void *data, zbncp_size_t size)
{
  const zbncp_uint8_t *p = (const zbncp_uint8_t *) data;
  zbncp_uint8_t crc = (zbncp_uint8_t) ~init;
  zbncp_size_t i;
  zbncp_size_t bit;

  for (i = 0u; i < size; ++i)
  {
    crc ^= p[i];
    for (bit = 0u; bit < 8u; ++bit)
    {
      zbncp_uint8_t bitval = (crc & 1u);
      zbncp_uint8_t mask = (zbncp_uint8_t) (-(zbncp_int16_t)bitval);
      /* Lowest bit was converted to a mask: 0 -> 0x00, 1 -> 0xFF */
      crc = (crc >> 1) ^ (ZBNCP_CRC8_GEN_POLY & mask);
    }
  }
  crc = (zbncp_uint8_t) ~crc;

  ZBNCP_DBG_TRACE("CRC8 init %#02x data %#p size %zu -> crc %#02x", init, data, size, crc);

  return crc;
}

zbncp_uint16_t zbncp_crc16(zbncp_uint16_t init, const void *data, zbncp_size_t size)
{
  const zbncp_uint8_t *p = (const zbncp_uint8_t *) data;
  zbncp_uint16_t crc = init;
  zbncp_size_t i;
  zbncp_size_t bit;

  for (i = 0u; i < size; ++i)
  {
    zbncp_uint16_t byte = p[i];
    crc ^= byte;
    for (bit = 0u; bit < 8u; ++bit)
    {
      zbncp_uint16_t bitval = (crc & 1u);
      zbncp_uint16_t mask = ((zbncp_uint16_t) -(zbncp_int32_t)bitval);
      /* Lowest bit was converted to a mask: 0 -> 0x0000, 1 -> 0xFFFF */
      crc = (crc >> 1) ^ (ZBNCP_CRC16_GEN_POLY & mask);
    }
  }

  ZBNCP_DBG_TRACE("CRC16 init %#04x data %#p size %zu --> crc %#04x", init, data, size, crc);

  return crc;
}
