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
/* PURPOSE: 24 bit variable mathematics
*/

#define ZB_TRACE_FILE_ID 1843


#include "zb_common.h"
#include "zb_types.h"

#ifdef ZB_UINT24_48_SUPPORT

#ifdef DEBUG

/* empty lines to fix ZB_ASSERT_COMPILE_DECL implementation...
 */

ZB_ASSERT_COMPILE_DECL(sizeof(zb_uint48_t) == sizeof(zb_int48_t));
ZB_ASSERT_COMPILE_DECL(sizeof(zb_uint48_t) == ZB_48BIT_SIZE);

#endif /* DEBUG */


void zb_uint64_to_uint48(zb_uint64_t var, zb_uint48_t *res)
{
  res->low = (zb_uint32_t)var;
  res->high = (zb_uint16_t)(var >> 32);
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint48_add(const zb_uint48_t *f, const zb_uint48_t *s, zb_uint48_t *r)
{
  zb_uint64_t temp = (zb_uint64_t)(f->low) + (zb_uint64_t)(s->low);

  temp += ((zb_uint64_t)f->high + (zb_uint64_t)s->high) << 32;

  if (((temp >> 48) & 1U) != 0U)
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_uint64_to_uint48(temp, r);

  return ZB_MATH_OK;
}


zb_int64_t zb_int48_to_int64(const zb_int48_t *var)
{
  zb_uint64_t res = 0;

  res |= var->low;
  res |= ((zb_uint64_t)(var->high) << 32);

  if (((zb_uint16_t)var->high & 0x8000U) != 0U)
  {
    res |= 0xFFFF000000000000U;
  }

  return (zb_int64_t)res;
}


void zb_int64_to_int48(zb_int64_t var, zb_int48_t *res)
{
  if ((var <= MAX_SIGNED_48BIT_VAL) && (var >= MIN_SIGNED_48BIT_VAL))
  {
    zb_uint48_t temp;

    temp.low = (zb_uint32_t)var;
    temp.high = (zb_uint16_t)((zb_uint64_t)var >> 32);

    res->low = temp.low;
    res->high = (zb_int16_t)temp.high;
  }
  else
  {
    /**
     * The variable type zb_int64_t can be out of the bounds
     */
    zb_uint64_t min_max_value = (zb_uint64_t)((var > MAX_SIGNED_48BIT_VAL) ? MAX_SIGNED_48BIT_VAL : MIN_SIGNED_48BIT_VAL);
    zb_uint16_t temp_high;

    res->low = (zb_uint32_t)min_max_value;

    temp_high = (zb_uint16_t)(min_max_value >> 32);
    res->high = (zb_int16_t)temp_high;
  }
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int48_add(const zb_int48_t *f, const zb_int48_t *s, zb_int48_t *r)
{

  zb_int64_t temp;

  temp = zb_int48_to_int64(f) + zb_int48_to_int64(s);

  if ((temp < MIN_SIGNED_48BIT_VAL) || (temp > MAX_SIGNED_48BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int48_sub(const zb_int48_t *f, const zb_int48_t *s, zb_int48_t *r)
{

  zb_int64_t temp;

  temp = zb_int48_to_int64(f) - zb_int48_to_int64(s);

  if ((temp < MIN_SIGNED_48BIT_VAL) || (temp > MAX_SIGNED_48BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}


void zb_int64_to_uint48(zb_int64_t var, zb_uint48_t *res)
{
  if (var <= (zb_int64_t)MAX_UNSIGNED_48BIT_VAL && (var >= 0))
  {
    res->low = (zb_uint32_t)var;
    res->high = (zb_uint16_t)((zb_uint64_t)var >> 32);
  }
  else
  {
    /**
     * The variable type zb_int64_t can be out of the bounds
     */
    zb_uint64_t min_max_value = (var < 0) ? 0U : MAX_UNSIGNED_48BIT_VAL;
    zb_uint16_t temp_high;

    res->low = (zb_uint32_t)min_max_value;

    temp_high = (zb_uint16_t)(min_max_value >> 32);
    res->high = temp_high;
  }
}


zb_int64_t zb_uint48_to_int64(const zb_uint48_t *var)
{
  zb_uint64_t res = 0;

  res |= var->low;
  res |= ((zb_uint64_t)var->high << 32);

  return (zb_int64_t)res;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint48_sub(const zb_uint48_t *f, const zb_uint48_t *s, zb_uint48_t *r)
{
  zb_uint8_t ret = ZB_MATH_OK;
  zb_int64_t temp;
  zb_int64_t temp_f = (zb_int64_t)zb_uint48_to_int64(f);
  zb_int64_t temp_s = (zb_int64_t)zb_uint48_to_int64(s);

  if (temp_f >= temp_s)
  {
    temp = temp_f - temp_s;
    zb_int64_to_uint48(temp, r);
  }
  else
  {
    ret = ZB_MATH_OVERFLOW;
  }

  return ret;
}


zb_uint8_t zb_int48_neg(const zb_int48_t *f, zb_int48_t *r)
{
  zb_int64_t temp;

  zb_uint64_t u64_temp = (zb_uint64_t)zb_int48_to_int64(f);
  u64_temp = ~(u64_temp) + 1U;

  temp = (zb_int64_t)u64_temp;

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint48_mul(const zb_uint48_t *f, const zb_uint48_t *s, zb_uint48_t *r)
{

  zb_uint64_t temp;

  temp = (zb_uint64_t)zb_uint48_to_int64(f) * (zb_uint64_t)zb_uint48_to_int64(s);

  if ((temp >> 48) != 0U)
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_uint64_to_uint48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int48_mul(const zb_int48_t *f, const zb_int48_t *s, zb_int48_t *r)
{

  zb_int64_t temp;

  temp = (zb_int64_t)zb_int48_to_int64(f) * (zb_int64_t)zb_int48_to_int64(s);

  if ((temp < MIN_SIGNED_48BIT_VAL) || (temp > MAX_SIGNED_48BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint48_div(const zb_uint48_t *f, const zb_uint48_t *s, zb_uint48_t *r)
{

  zb_int64_t temp;

  temp = (zb_int64_t)zb_uint48_to_int64(f) / (zb_int64_t)zb_uint48_to_int64(s);

  zb_int64_to_uint48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int48_div(const zb_int48_t *f, const zb_int48_t *s, zb_int48_t *r)
{

  zb_int64_t temp;

  temp = (zb_int64_t)zb_int48_to_int64(f) / (zb_int64_t)zb_int48_to_int64(s);

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint48_mod(const zb_uint48_t *f, const zb_uint48_t *s, zb_uint48_t *r)
{

  zb_uint64_t temp;

  temp = (zb_uint64_t)zb_uint48_to_int64(f) % (zb_uint64_t)zb_uint48_to_int64(s);

  zb_uint64_to_uint48(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int48_mod(const zb_int48_t *f, const zb_int48_t *s, zb_int48_t *r)
{

  zb_int64_t temp;

  temp = (zb_int64_t)zb_int48_to_int64(f) % (zb_int64_t)zb_int48_to_int64(s);

  zb_int64_to_int48(temp, r);

  return ZB_MATH_OK;
}

#endif /* ZB_UINT24_48_SUPPORT */
