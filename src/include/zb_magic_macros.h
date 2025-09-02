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
/* PURPOSE: Some macros for fast calculation

*/
#ifndef MAGIC_MACROSES_H
#define MAGIC_MACROSES_H 1

/*! @cond internals_doc */
/**
   @addtogroup ZB_BASE
   @{
*/

/**
 * These algorithms are published by Henry Gordon Dietz (University of Kentucky)
 * in the article "The Aggregate Magic Algorithms".
 *
 * See: [The Aggregate Magic Algorithms](aggregate.org/MAGIC/)
 */

#define MAGIC_ROUND_TO_4(a) (((a) + 3U) / 4U * 4U)

#define MAGIC_TRUNC_TO_8(a) ((a) & (~7U))

#define MAGIC_LEAST_SIGNIFICANT_BIT_MASK(x) ((x) & (zb_uint_t)(-((zb_int_t)(x))))

/**
   Return 1 if the number is a power of 2, works only if x > 0
 */
#define MAGIC_IS_POWER_OF_TWO(x) ( ((x) & ((x) - 1U)) == 0U )


/**
 * Following set of macros implements compile-time calculation of # of bits in the word.
 *
 * See: [The Aggregate Magic Algorithms: Population Count](aggregate.org/MAGIC/#Population%20Count%20(Ones%20Count))
 */

#define MAGIC_ONE_1(x) ((x) - (((x) >> 1U) & 0x55555555U))
#define MAGIC_ONE_2(x) (((MAGIC_ONE_1(x) >> 2U) & 0x33333333U) + (MAGIC_ONE_1(x) & 0x33333333U))
#define MAGIC_ONE_4(x) (((MAGIC_ONE_2(x) >> 4U) + MAGIC_ONE_2(x)) & 0x0f0f0f0fU)
#define MAGIC_ONE_8(x) (MAGIC_ONE_4(x) + (MAGIC_ONE_4(x) >> 8U))
#define MAGIC_ONE_16(x) (MAGIC_ONE_8(x) + (MAGIC_ONE_8(x) >> 16U))
#define MAGIC_ONE_32(x) (MAGIC_ONE_16(x) & 0x0000003fU)


#define MAGIC_LOG2_1(x) ((x) | ((x) >> 1U))
#define MAGIC_LOG2_2(x) (MAGIC_LOG2_1(x) | (MAGIC_LOG2_1(x) >> 2U))
#define MAGIC_LOG2_4(x) (MAGIC_LOG2_2(x) | (MAGIC_LOG2_2(x) >> 4U))
#define MAGIC_LOG2_8(x) (MAGIC_LOG2_4(x) | (MAGIC_LOG2_4(x) >> 8U))
#define MAGIC_LOG2_16(x) (MAGIC_LOG2_8(x) | (MAGIC_LOG2_8(x) >> 16U))

#define MAGIC_LOG2_32(x) (MAGIC_ONE_32(MAGIC_LOG2_16(x)) - 1U)

/*! @} */
/*! @endcond */

#endif /* MAGIC_MACROSES_H */
