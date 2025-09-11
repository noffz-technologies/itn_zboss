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
/*  PURPOSE: NCP utility functions declaration.
*/
#ifndef ZBNCP_INCLUDE_GUARD_UTILS_H
#define ZBNCP_INCLUDE_GUARD_UTILS_H 1

#include "zbncp_types.h"

#define ZBNCP_CRC8_INIT_VAL   0x00u   /**< Initial value to use for CRC8 calculation */
#define ZBNCP_CRC16_INIT_VAL  0x0000u /**< Initial value to use for CRC16 calculation */

/**
 * @brief Calculate CRC8 of the data buffer starting from initial CRC8 value.
 *
 * @param init - initial CRC8 value
 * @param data - constant pointer to the data buffer
 * @param size - size of the data buffer
 *
 * @return CRC8 of the data buffer
 */
zbncp_uint8_t zbncp_crc8(zbncp_uint8_t init, const void *data, zbncp_size_t size);

/**
 * @brief Calculate CRC16 of the data buffer starting from initial CRC16 value.
 *
 * @param init - initial CRC16 value
 * @param data - constant pointer to the data buffer
 * @param size - size of the data buffer
 *
 * @return CRC16 of the data buffer
 */
zbncp_uint16_t zbncp_crc16(zbncp_uint16_t init, const void *data, zbncp_size_t size);

/**
 * @brief Determine the minimum size value of the two.
 *
 * @param lhs - left hand side value to compare
 * @param lhs - right hand side value to compare
 *
 * @return minimum of the two
 */
static inline zbncp_size_t size_min(zbncp_size_t lhs, zbncp_size_t rhs)
{
  return ((lhs < rhs) ? lhs : rhs);
}

/**
 * @brief Determine the maximum size value of the two.
 *
 * @param lhs - left hand side value to compare
 * @param lhs - right hand side value to compare
 *
 * @return maximum of the two
 */
static inline zbncp_size_t size_max(zbncp_size_t lhs, zbncp_size_t rhs)
{
  return ((lhs > rhs) ? lhs : rhs);
}

#endif /* ZBNCP_INCLUDE_GUARD_UTILS_H */
