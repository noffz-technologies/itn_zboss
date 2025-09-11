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
/*  PURPOSE: Generic memory copying/moving functions implementation.
*/

#define ZB_TRACE_FILE_ID 30
#include "zbncp_mem.h"
#include "zbncp_debug.h"

ZBNCP_DBG_STATIC_ASSERT(sizeof(void*) <= sizeof(zbncp_uintptr_t))

#if !ZBNCP_USE_STDMEM

static void zbncp_mem_copy_forward(void *dest, const void *src, zbncp_size_t size)
{
  zbncp_size_t i;
  char *pd = (char *)dest;
  const char *ps = (const char *)src;
  for (i = 0u; i < size; ++i)
  {
    pd[i] = ps[i];
  }
}

static void zbncp_mem_copy_backward(void *dest, const void *src, zbncp_size_t size)
{
  zbncp_size_t i;
  char *pd = (char *)dest;
  const char *ps = (const char *)src;
  if (size != 0u)
  {
    for (i = 0u; i < size; ++i)
    {
      zbncp_size_t ri = size - i - 1u;
      pd[ri] = ps[ri];
    }
  }
}

void zbncp_mem_copy(void *dest, const void *src, zbncp_size_t size)
{
  zbncp_mem_copy_forward(dest, src, size);
}

void zbncp_mem_move(void *dest, const void *src, zbncp_size_t size)
{
  /* MISRA-C 2012 Rule 18.3 prohibits to use relational operators
   * (>, >=, <, <=) to objects of pointer type. We need to detect
   * memory copying direction so we cast pointers to appropriate-
   * sized integers and compare them. */
  zbncp_uintptr_t pd = (zbncp_uintptr_t)(char *)dest;
  zbncp_uintptr_t ps = (zbncp_uintptr_t)(const char *)src;

  if (pd < ps)
  {
    zbncp_mem_copy_forward(dest, src, size);
  }
  else
  {
    zbncp_mem_copy_backward(dest, src, size);
  }
}

void zbncp_mem_fill(void *mem, zbncp_size_t size, zbncp_uint8_t val)
{
  zbncp_size_t i;
  char *p = (char *)mem;
  for (i = 0u; i < size; ++i)
  {
    p[i] = (char)val;
  }
}

#endif /* !ZBNCP_USE_STDMEM */
