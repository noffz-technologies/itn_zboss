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
/* PURPOSE: Random number generator and auxiliary functions (CRC, hash, etc.)
*/

#define ZB_TRACE_FILE_ID 123
#include "zboss_api_core.h"
#include "zb_magic_macros.h"
#include "zb_hash.h"

static zb_uint32_t zb_last_random_value_jtr = 0;

#ifndef ZB_CONTEXTS_IN_SEPARATE_FILE

#include "zb_common.h"
#include "zb_common_u.h"

/* Interrupt context. Moved here to build utils */
#ifdef ZB_CONFIG_FIX_G_IZB
/* fixing address the g_izb in memory,
otherwise the addresses of the SPI buffers may changed in different builds */
__attribute__((section (".zb_global"))) zb_intr_globals_t g_izb;
#else
zb_intr_globals_t g_izb;
#endif
#endif /* ZB_CONTEXTS_IN_SEPARATE_FILE */

/* Fowler-Noll-Vo 32 bit offset basis */
#define FNV1_32_INIT ((zb_uint32_t)0x811c9dc5U)
/* Fowler-Noll-Vo 32 bit prime value */
#define FNV_32_PRIME ((zb_uint32_t)0x01000193U)


zb_uint32_t zb_random()
{
  return zb_random_seed();
}

zb_uint32_t zb_random_val(zb_uint32_t max_value)
{
  zb_uint32_t ret = (max_value == 0U) ? 0U : (zb_random() / (ZB_RAND_MAX / max_value));
  ZB_ASSERT(ret <= max_value);
  return ret;
}

zb_uint32_t zb_random_jitter()
{
  if (zb_last_random_value_jtr == 0U)
  {
    zb_last_random_value_jtr = zb_random_seed();
  }
  zb_last_random_value_jtr = 1103515245UL * zb_last_random_value_jtr + 12345UL;

  return zb_last_random_value_jtr;
}

#ifndef ZB_LITTLE_ENDIAN
void zb_htole16(zb_uint8_t ZB_XDATA *ptr, zb_uint8_t ZB_XDATA *val)
{
  ptr[1] = val[0];
  ptr[0] = val[1];
}
void zb_htole32(zb_uint8_t ZB_XDATA *ptr, zb_uint8_t ZB_XDATA *val)
{
  ptr[3] = val[0],
  ptr[2] = val[1],
  ptr[1] = val[2],
  ptr[0] = val[3];
}
#else
void zb_htobe32(zb_uint8_t ZB_XDATA *ptr, zb_uint8_t ZB_XDATA *val)
{
  ((zb_uint8_t *)(ptr))[3] = ((zb_uint8_t *)(val))[0],
  ((zb_uint8_t *)(ptr))[2] = ((zb_uint8_t *)(val))[1],
  ((zb_uint8_t *)(ptr))[1] = ((zb_uint8_t *)(val))[2],
  ((zb_uint8_t *)(ptr))[0] = ((zb_uint8_t *)(val))[3];
}
#endif


#ifndef ZB_IAR
void zb_get_next_letoh16(zb_uint16_t *dst, zb_uint8_t **src)
{
  ZB_LETOH16(dst, *src);
  (*src) += 2;
}
#endif

void* zb_put_next_htole16(zb_uint8_t *dst, zb_uint16_t val)
{
  ZB_HTOLE16(dst, &val);
  return (dst + 2);
}

void* zb_put_next_2_htole16(zb_uint8_t *dst, zb_uint16_t val1, zb_uint16_t val2)
{
  ZB_HTOLE16(dst, &val1);
  ZB_HTOLE16(dst+2, &val2);
  return (void*)(dst+4);
}

void* zb_put_next_2_htole32(zb_uint8_t *dst, zb_uint32_t val1, zb_uint32_t val2)
{
  ZB_HTOLE32(dst, &val1);
  ZB_HTOLE32((dst + 4), &val2);
  return (void*)(dst + 8);
}

void* zb_put_next_htole32(zb_uint8_t *dst, zb_uint32_t val1)
{
  ZB_HTOLE32(dst, &val1);
  return (void*)(dst + 4);
}

/**
 * Calculate Fowler-Noll-Vo hash.
 * FNV hash algorithms and source code have been released into the public domain.
 * The authors of the FNV algorithm took deliberate steps to disclose the algorithm
 * in a public forum soon after it was invented. More than a year passed after this
 * public disclosure and the authors deliberately took no steps to patent the FNV algorithm.
 * Therefore it is safe to say that the FNV authors have no patent claims on the FNV algorithm as published.
 *
 * See: [FNV Hash](www.isthe.com/chongo/tech/comp/fnv/)
 */
zb_uint32_t zb_fnv_32a_uint16(zb_uint16_t v)
{
  zb_uint32_t hval = FNV1_32_INIT;

  /* xor the bottom with the current octet */
  hval ^= (zb_uint32_t)(v) & 0xFFU;
  /* multiply by the 32 bit FNV magic prime mod 2^32 */
  hval += (hval<<1U) + (hval<<4U) + (hval<<7U) + (hval<<8U) + (hval<<24U);
  v >>= 8U;
  hval ^= (zb_uint32_t)(v);
  /* multiply by the 32 bit FNV magic prime mod 2^32 */
  hval += (hval<<1U) + (hval<<4U) + (hval<<7U) + (hval<<8U) + (hval<<24U);
  return hval;
}

/**
 * Calculate CRC-8 checksum using classic algorithm based on polynomial arithmetic.
 * In the current implementation we use the polynomial x^8+x^6+x^3+x^2+1.
 * The polynomial is chosen based on the article "Cyclic Redundancy Code Polynomial
 * Selection For Embedded Networks" (Philip Koopman/Tridib Chakravarty)
 * as one of the best published polynomial for general purposes.
 * Polynomial representation: 0x4D - normal; 0xB2 - reverse; 0xA6 - in Koopman's notation.
 * This implementation is reflected (shifts to the right) and based on the reversed polynomial.
 * The description of this CRC is the following:
 * width=8 poly=0x4d init=0xff refin=true refout=true xorout=0xff check=0xd8 name="CRC-8/KOOP"
 *
 * See: Hacker's Delight. Chapter 14. Cyclic Redundancy Check.
 *      [Cyclic redundancy check](en.wikipedia.org/wiki/Cyclic_redundancy_check)
 *      [A painless guide to CRC error detection algorithms](zlib.net/crc_v3.txt)
 *      [CRC Polynomial Selection For Embedded Networks](users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf)
 */
zb_uint8_t zb_crc8(const zb_uint8_t *p, zb_uint8_t crc, zb_uint_t len)
{
  zb_uint_t i, j;

  ZB_ASSERT(p != NULL);

  crc = ~crc & 0xFFU;

  for (i = 0U; i < len; ++i)
  {
    crc ^= p[i];

    for (j = 0U; j < 8U; j++)
    {
      if (ZB_U2B(crc & 1U))
      {
        crc = (crc >> 1) ^ 0xB2U;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc ^ 0xFFU;
}

/**
 * Calculate CRC-16 checksum using classic algorithm based on polynomial arithmetic.
 * In the current implementation we use the polynomial x^16+x^12+x^5+x^2+1 (CRC-16-CCITT).
 * Polynomial representation: 0x1021 - normal; 0x8408 - reverse.
 * This implementation is reflected (shifts to the right) and based on the reversed polynomial.
 * Note: do not inverse only for compatibility with apps,
 * in the standard CRC-16/X-25 config, the input CRC must be fixed to 0xffff and output must be inverted!
 *
 * See: Hacker's Delight. Chapter 14. Cyclic Redundancy Check.
 *      [Cyclic redundancy check](en.wikipedia.org/wiki/Cyclic_redundancy_check)
 *      [A painless guide to CRC error detection algorithms](zlib.net/crc_v3.txt)
 */
zb_uint16_t zb_crc16(const zb_uint8_t *p, zb_uint16_t crc, zb_uint_t len)
{
  zb_uint_t i, j;

  ZB_ASSERT(p != NULL);

  for (i = 0U; i < len; ++i)
  {
    crc ^= p[i];

    for (j = 0U; j < 8U; j++)
    {
      if (ZB_U2B(crc & 0x0001U))
      {
        crc = (crc >> 1) ^ 0x8408U;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}


/**
 * Calculate CRC-32 checksum using classic algorithm based on polynomial arithmetic.
 * In the current implementation we use the polynomial x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 (CRC-32).
 * Polynomial representation: 0x04C11DB7 - normal; 0xEDB88320 - reverse.
 * This implementation is reflected (shifts to the right) and based on the reversed polynomial.
 *
 * See: Hacker's Delight. Chapter 14. Cyclic Redundancy Check.
 *      [Cyclic redundancy check](en.wikipedia.org/wiki/Cyclic_redundancy_check)
 *      [A painless guide to CRC error detection algorithms](zlib.net/crc_v3.txt)
 */
zb_uint32_t zb_crc32(const zb_uint8_t *p, zb_uint_t len)
{
  zb_uint_t i, j;
  zb_uint32_t crc, mask;

  ZB_ASSERT(p != NULL);

  /* Initialize CRC with fixed value */
  crc = 0xFFFFFFFFUL;

  for (i = 0U; i < len; ++i)
  {
    crc = crc ^ p[i];
    for (j = 0U; j < 8U; j++)
    {
      mask = ZB_BIT_IS_SET(crc, ZB_BITS1(0)) ? ~(0U) : 0U;
      crc = (crc >> 1) ^ (0xEDB88320UL & mask);
    }
  }

  return ~crc;
}

/**
 * Calculate CRC-32 checksum using classic algorithm based on polynomial arithmetic.
 * In the current implementation we use the polynomial x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 (CRC-32).
 * This implementation is reflected (shifts to the right) and based on the reversed polynomial.
 * Note: this function is almost the same as zb_crc32 but with different input arguments.
 */
zb_uint32_t zb_crc32_next(zb_uint32_t prev_crc, zb_uint8_t *p, zb_uint_t len)
{
  zb_uint_t i, j;
  zb_uint32_t crc, mask;

  ZB_ASSERT(p != NULL);

  /* Initialize CRC with inverted value */
  crc = ~prev_crc;

  for (i = 0U; i < len; ++i)
  {
    crc = crc ^ p[i];
    for (j = 0U; j < 8U; j++)
    {
      mask = ZB_BIT_IS_SET(crc, ZB_BITS1(0)) ? ~(0U) : 0U;
      crc = (crc >> 1) ^ (0xEDB88320UL & mask);
    }
  }

  return ~crc;
}

/**
 * Calculate CRC-32 checksum using classic algorithm based on polynomial arithmetic.
 * In the current implementation we use the polynomial x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1 (CRC-32).
 * This implementation is normal (shifts to the left) and based on the forward polynomial.
 * Note: this implementation is incorrect and produces CRC values that don't match with zb_crc32 function.
 * We keep it here for backward compatibility with Production config v1 only.
 */
zb_uint32_t zb_crc32_next_v2(zb_uint32_t prev_crc, zb_uint8_t *p, zb_uint16_t len)
{
#define ZB_CRC32_POLY 0x04C11DB7U
  zb_uint32_t crc, c, i, j;

  ZB_ASSERT(p != NULL);

  crc = (ZB_U2B(prev_crc)) ? (~prev_crc) : 0U;

  for (i = 0U; i < len; ++i)
  {
    c  =  (((crc ^ p[i]) & 0xFFU) << 24);
    for (j = 0U; j < 8U; j++)
    {
      if (ZB_U2B(c & 0x80000000U))
      {
        c = (c << 1) ^ ZB_CRC32_POLY;
      }
      else
      {
        c <<= 1;
      }
    }
    crc = (crc >> 8) ^ c;
  }

  return ~crc;
}

/**
 * Calculate a hash using DJB2 hash function.
 * This algorithm was first reported by Dan Bernstein many years ago in comp.lang.c.
 * Another version of this algorithm (now favored by Bernstein) uses XOR:
 * hash(i) = hash(i - 1) * 33 ^ str[i];
 * The magic of number 33 (why it works better than many other constants, prime or not)
 * has never been adequately explained.
 * The algorithm is in the public domain.
 *
 * See: [Hash Functions](www.cse.yorku.ca/~oz/hash.html)
 */
zb_uint32_t zb_64bit_hash(zb_uint8_t *data)
{
  zb_uint_t i;
  zb_uint32_t hash = ZB_HASH_MAGIC_VAL;

  for (i = 0 ; i < sizeof(zb_64bit_addr_t) ; ++i)
  {
    hash = ZB_HASH_FUNC_STEP(hash, data[i]);
  }

  return (hash & ZB_INT_MASK);
}

#ifndef ZB_CONFIG_NRF52
void zb_bzero(void *p, zb_uint_t size)
{
  ZB_MEMSET(p, 0, size);
}

void zb_bzero_2(void *p)
{
  ZB_MEMSET(p, 0, 2);
}
#endif


void zb_bzero_volatile(volatile void *s, zb_uint_t size)
{
  volatile zb_uint8_t *p = s;
  while (size-- > 0U)
  {
    p[size] = 0;
  }
}


void zb_memcpy8(void *vptr, void *vsrc)
{
  zb_uint_t i;
  zb_uint8_t *ptr = vptr;
  zb_uint8_t *src = vsrc;
  for (i = 0U; i < 8U; ++i)
  {
    *ptr ++= *src++;
  }
}

/**
 * Calculates number of '1' in 16bit bitmask using the table
 * that contains numbers of bits per 4-bit fragment.
 * Note: this function can also be implemented using magic macros
 * MAGIC_ONE_8(a) & 0x1FU.
 *
 * See: Hacker's Delight. Chapter 5. Counting Bits.
 */
zb_uint8_t zb_bit_cnt16(zb_uint16_t a)
{
  static const zb_uint8_t bitc[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
  return bitc[ a & 0x0FU ] + bitc[ (a>>4) & 0x0FU ] + bitc[ (a>>8) & 0x0FU ] + bitc[ (a>>12) & 0x0FU ];
}


void zb_inverse_bytes(zb_uint8_t *ptr, zb_uint32_t len)
{
  zb_uint8_t tmp;
  zb_uint32_t i;

  for(i=0U; i < len / 2U; i++)
  {
    tmp = *(ptr+i);
    *(ptr+i) = *(ptr+len-i-1);
    *(ptr+len-i-1) = tmp;
  }
}


zb_uint_t zb_high_bit_number(zb_uint32_t mask)
{
  return MAGIC_LOG2_32(mask);
}


zb_uint_t zb_low_bit_number(zb_uint32_t mask)
{
  zb_uint32_t v = MAGIC_LEAST_SIGNIFICANT_BIT_MASK(mask);
  return zb_high_bit_number(v);
}

/**
 * Generate Pseudorandom binary sequence (PRBS-9) using linear-feedback shift registers.
 * The algorithm is based on the LFSR algorithm that uses PRBS-9 sequence generating monic polynomial.
 * PRBS-9 polynomial is x^9+x^5+1.
 * The current implementation is intended to set randomly generated bits in the provided buffer.
 *
 * See: [Pseudorandom binary sequence](en.wikipedia.org/wiki/Pseudorandom_binary_sequence)
 *      [Linear-feedback shift register](en.wikipedia.org/wiki/Linear-feedback_shift_register)
 */
void zb_generate_prbs9(zb_uint8_t *dest, zb_uint16_t cnt, zb_uint16_t seed)
{
  zb_uint16_t lfsr = seed;
  zb_uint16_t i = 0;
  zb_uint16_t new_bit;

  ZB_ASSERT(dest != NULL);

  do
  {
    new_bit = ((lfsr >> 8U) ^ (lfsr >> 4U)) & 1U;
    lfsr = ((lfsr << 1U) & 0xFFFEU) | new_bit;
    if (ZB_U2B(new_bit))
    {
      ZB_SET_BIT_IN_BIT_VECTOR(dest, i);
    }
    else
    {
      ZB_CLR_BIT_IN_BIT_VECTOR(dest, i);
    }
    ++i;
  }
  while(i < cnt * 8U);
}
