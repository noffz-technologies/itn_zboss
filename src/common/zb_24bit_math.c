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

#define ZB_TRACE_FILE_ID 12299


#include "zb_common.h"
#include "zb_types.h"

#ifdef ZB_UINT24_48_SUPPORT

#ifdef DEBUG

/* empty lines to fix ZB_ASSERT_COMPILE_DECL implementation...
 */
ZB_ASSERT_COMPILE_DECL(sizeof(zb_uint24_t) == sizeof(zb_int24_t));
ZB_ASSERT_COMPILE_DECL(sizeof(zb_uint24_t) == ZB_24BIT_SIZE);

#endif /* DEBUG */


void zb_uint32_to_uint24(zb_uint32_t var, zb_uint24_t *res)
{
  res->low = (zb_uint16_t)var;
  res->high = (zb_uint8_t)(var >> 16);
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint24_add(const zb_uint24_t *f, const zb_uint24_t *s, zb_uint24_t *r)
{
  zb_uint32_t temp = (zb_uint32_t)(f->low) + (zb_uint32_t)(s->low);

  temp += ((zb_uint32_t)f->high + (zb_uint32_t)s->high) << 16;

  if (((temp>>24) & 1U) != 0U)
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_uint32_to_uint24(temp, r);

  return ZB_MATH_OK;
}


zb_int32_t zb_int24_to_int32(const zb_int24_t *var)
{
  zb_uint32_t res = 0;

  res |= var->low;
  res |= ((zb_uint32_t)var->high << 16);

  if (((zb_uint16_t)var->high & 0x80U) != 0U)
  {
    res |= 0xFF000000U;
  }

  return (zb_int32_t)res;
}


void zb_int32_to_int24(zb_int32_t var, zb_int24_t *res)
{
  if ((var <= MAX_SIGNED_24BIT_VAL) && (var >= MIN_SIGNED_24BIT_VAL))
  {
    zb_uint24_t temp;

    temp.low = (zb_uint16_t)var;
    temp.high = (zb_uint8_t)((zb_uint32_t)var >> 16);

    res->low = temp.low;
    res->high = (zb_int8_t)temp.high;
  }
  else
  {
    /**
     * The variable type zb_int32_t can be out of the bounds
     */
    zb_uint32_t min_max_value = (zb_uint32_t)((var > MAX_SIGNED_24BIT_VAL) ? MAX_SIGNED_24BIT_VAL : MIN_SIGNED_24BIT_VAL);
    zb_uint8_t temp_high;

    res->low = (zb_uint16_t)min_max_value;

    temp_high = (zb_uint8_t)(min_max_value >> 16);
    res->high = (zb_int8_t)temp_high;
  }
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_add(const zb_int24_t *f, const zb_int24_t *s, zb_int24_t *r)
{

  zb_int32_t temp;

  temp = zb_int24_to_int32(f) + zb_int24_to_int32(s);

  if ((temp < MIN_SIGNED_24BIT_VAL) || (temp > MAX_SIGNED_24BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_sub(const zb_int24_t *f, const zb_int24_t *s, zb_int24_t *r)
{

  zb_int32_t temp;

  /*TODO: check cases x1 - x2, (-x1) - (-x2), (-x1) - (x2), (x1) - (-x1) */
  temp = zb_int24_to_int32(f) - zb_int24_to_int32(s);

  if ((temp < MIN_SIGNED_24BIT_VAL) || (temp > MAX_SIGNED_24BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}


void zb_int32_to_uint24(zb_int32_t var, zb_uint24_t *res)
{
  if (var <= (zb_int32_t)MAX_UNSIGNED_24BIT_VAL && (var >= 0))
  {
    res->low = (zb_uint16_t)var;
    res->high = (zb_uint8_t)((zb_uint32_t)var >> 16);
  }
  else
  {
    /**
     * The variable type zb_int32_t can be out of the bounds
     */
    zb_uint32_t min_max_value = (var < 0) ? 0U : MAX_UNSIGNED_24BIT_VAL;
    zb_uint8_t temp_high;

    res->low = (zb_uint16_t)min_max_value;

    temp_high = (zb_uint8_t)(min_max_value >> 16);
    res->high = temp_high;
  }
}


zb_int32_t zb_uint24_to_int32(const zb_uint24_t *var)
{
  zb_uint32_t res = 0;

  res |= var->low;
  res |= ((zb_uint32_t)var->high << 16);

  return (zb_int32_t)res;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint24_sub(const zb_uint24_t *f, const zb_uint24_t *s, zb_uint24_t *r)
{
  zb_uint8_t ret = ZB_MATH_OK;
  zb_int32_t temp;
  zb_int32_t temp_f = (zb_int32_t)zb_uint24_to_int32(f);
  zb_int32_t temp_s = (zb_int32_t)zb_uint24_to_int32(s);

  if (temp_f >= temp_s)
  {
    temp = temp_f - temp_s;
    zb_int32_to_uint24(temp, r);
  }
  else
  {
    ret = ZB_MATH_OVERFLOW;
  }

  return ret;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_neg(const zb_int24_t *f, zb_int24_t *r)
{
  zb_int32_t temp;

  zb_uint32_t u32_temp = (zb_uint32_t)zb_int24_to_int32(f);
  u32_temp = ~(u32_temp) + 1U;

  temp = (zb_int32_t)u32_temp;

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint24_mul(const zb_uint24_t *f, const zb_uint24_t *s, zb_uint24_t *r)
{

  zb_uint32_t temp;

  temp = (zb_uint32_t)zb_uint24_to_int32(f) * (zb_uint32_t)zb_uint24_to_int32(s);

  if ((temp >> 24) != 0U)
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_uint32_to_uint24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_mul(const zb_int24_t *f, const zb_int24_t *s, zb_int24_t *r)
{

  zb_int32_t temp;

  temp = (zb_int32_t)zb_int24_to_int32(f) * (zb_int32_t)zb_int24_to_int32(s);

  if ((temp < MIN_SIGNED_24BIT_VAL) || (temp > MAX_SIGNED_24BIT_VAL))
  {
    return ZB_MATH_OVERFLOW;
  }

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint24_div(const zb_uint24_t *f, const zb_uint24_t *s, zb_uint24_t *r)
{

  zb_int32_t temp;

  temp = (zb_int32_t)zb_uint24_to_int32(f) / (zb_int32_t)zb_uint24_to_int32(s);

  zb_int32_to_uint24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_div(const zb_int24_t *f, const zb_int24_t *s, zb_int24_t *r)
{

  zb_int32_t temp;

  temp = (zb_int32_t)zb_int24_to_int32(f) / (zb_int32_t)zb_int24_to_int32(s);

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_uint24_mod(const zb_uint24_t *f, const zb_uint24_t *s, zb_uint24_t *r)
{

  zb_uint32_t temp;

  temp = (zb_uint32_t)zb_uint24_to_int32(f) % (zb_uint32_t)zb_uint24_to_int32(s);

  zb_uint32_to_uint24(temp, r);

  return ZB_MATH_OK;
}


/**
 * @f - first operand
 * @s - second operand
 * @r - result of operation
 */
zb_uint8_t zb_int24_mod(const zb_int24_t *f, const zb_int24_t *s, zb_int24_t *r)
{

  zb_int32_t temp;

  temp = (zb_int32_t)zb_int24_to_int32(f) % (zb_int32_t)zb_int24_to_int32(s);

  zb_int32_to_int24(temp, r);

  return ZB_MATH_OK;
}

#endif /* ZB_UINT24_48_SUPPORT */
