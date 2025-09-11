/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2023 DSR Corporation, Denver CO, USA.
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
/* PURPOSE: Zigbee addresses routine
*/

/*! \addtogroup ZB_NWK_ADDR */
/*! @{ */

#define ZB_TRACE_FILE_ID 112
#include "zb_common.h"
#include "zb_hash.h"
#include "zb_address.h"
#include "zb_address_internal.h"

#ifndef ZB_MACSPLIT_DEVICE

static zb_bool_t ieee_search(zb_ieee_addr_compressed_t *ieee_compressed, zb_address_ieee_ref_t *ref_p);

static zb_bool_t short_search(zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p);

static zb_ret_t addr_add(zb_ieee_addr_compressed_t *ieee_compressed,
                         zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p);

static void del_short_sorted(zb_address_ieee_ref_t ref);

static void clock_tick(void);


zb_bool_t zb_check_bit_in_bit_vector(zb_uint8_t *v, zb_ushort_t b)
{
  return ZB_CHECK_BIT_IN_BIT_VECTOR(v, b);
}

void zb_set_bit_in_bit_vector(zb_uint8_t *v, zb_ushort_t b)
{
  ZB_SET_BIT_IN_BIT_VECTOR(v, b);
}

void zb_clr_bit_in_bit_vector(zb_uint8_t *v, zb_ushort_t b)
{
  ZB_CLR_BIT_IN_BIT_VECTOR(v, b);
}

#define ZB_ADDRESS_PAN_ID_HASH(pan_id) (\
  ZB_4INT_HASH_FUNC(\
    (zb_uint32_t)pan_id[0],\
    (zb_uint32_t)pan_id[2],\
    (zb_uint32_t)pan_id[4],\
    (zb_uint32_t)pan_id[6])\
    % ZB_PANID_TABLE_SIZE\
)

zb_ret_t zb_address_set_pan_id(zb_uint16_t short_pan_id, zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t *ref)
{
  zb_ret_t ret = RET_OK;
  zb_uint32_t hash_panid = ZB_ADDRESS_PAN_ID_HASH(pan_id);
  zb_address_pan_id_ref_t h_i = (zb_address_pan_id_ref_t)hash_panid;
  zb_address_pan_id_ref_t i;
  zb_address_pan_id_ref_t free_i = (zb_address_pan_id_ref_t)-1;
  zb_bool_t found = ZB_FALSE;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_add_pan_id short_pan_id %d pan_id " TRACE_FORMAT_64 " ref %p", (FMT__D_A_P,
           (int) short_pan_id, TRACE_ARG_64(pan_id), ref));

  /* try to find pan id first */
  TRACE_MSG(TRACE_COMMON3, "start search pan_id reference from h_i %hd", (FMT__H, h_i));
  i = h_i;
  do
  {
    if (zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, i))
    {
      if (ZB_EXTPANID_CMP(ZG->addr.pan_map[i].long_panid, pan_id) )
      {
        TRACE_MSG(TRACE_COMMON1, "pan_id reference found %d", (FMT__D, (int)i));
        if (ZG->addr.pan_map[i].short_panid != short_pan_id)
        {
          TRACE_MSG(TRACE_COMMON1, "updating short pan_id, seems it's changed", (FMT__0));
          ZG->addr.pan_map[i].short_panid = short_pan_id;
        }
        found = ZB_TRUE;
        break;
      }
    }
    else
    {
      free_i = i;
    }
    /* ZB_PANID_TABLE_SIZE is power of 2, so it is cheap to use division */
    /* i = ( i + 1 ) % ZB_PANID_TABLE_SIZE;
       if use sdcc, then code size increases by ~45 bytes  :)
    */
    i = i + 1U;
    i = i % ZB_PANID_TABLE_SIZE;
  } while ( i != h_i );

  /* if found return ref, otherwise add new pan_id */
  if ( found )
  {
    *ref = i;
    ret = RET_ALREADY_EXISTS;
  }
  else
  {
    /* try to find free place */
    if (free_i != (zb_address_pan_id_ref_t)-1
        /* Strict condition for prevention array borders violation */
        && free_i < ZB_ARRAY_SIZE(ZG->addr.pan_map))
    {
      TRACE_MSG(TRACE_COMMON1, "pan_id ref after find free place %d", (FMT__D, (int)free_i));
      ZB_EXTPANID_COPY(ZG->addr.pan_map[free_i].long_panid, pan_id);
      ZG->addr.pan_map[free_i].short_panid = short_pan_id;
      zb_set_bit_in_bit_vector(ZG->addr.used_pan_addr, free_i);
      *ref = free_i;
    }
    else
    {
      /* no free place */
      TRACE_MSG(TRACE_COMMON1, "pan address array is full", (FMT__0));
      ret = RET_OVERFLOW;
    }
  }

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_add_pan_id %d", (FMT__D, ret));
  return ret;
}


void zb_address_get_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id)
{
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id pan_id_reference %d pan_id %p", (FMT__D_P,(int) pan_id_ref, pan_id));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) == ZB_TRUE );
  ZB_EXTPANID_COPY(pan_id, ZG->addr.pan_map[pan_id_ref].long_panid);

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id", (FMT__0));
}


void zb_address_clear_pan_id_table(zb_ext_pan_id_t pan_id)
{
  zb_address_pan_id_ref_t panid_ref;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_clear_pan_id_table pan_id " TRACE_FORMAT_64,
            (FMT__A, TRACE_ARG_64(pan_id)));

  /* if found ref clear all used pan id except pan_id */
  if (zb_address_get_pan_id_ref(pan_id, &panid_ref) == RET_OK)
  {
    ZB_MEMSET(ZG->addr.used_pan_addr, 0x00, sizeof(ZG->addr.used_pan_addr));
    zb_set_bit_in_bit_vector(ZG->addr.used_pan_addr, panid_ref);
  }

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_clear_pan_id_table", (FMT__0));
}


void zb_address_reset_pan_id_table(void)
{
  ZB_MEMSET(ZG->addr.used_pan_addr, 0x00, sizeof(ZG->addr.used_pan_addr));
}


zb_ret_t zb_address_get_pan_id_ref(zb_ext_pan_id_t pan_id , zb_address_pan_id_ref_t *ref)
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_uint32_t hash_panid = ZB_ADDRESS_PAN_ID_HASH(pan_id);
  zb_address_pan_id_ref_t h_i = (zb_address_pan_id_ref_t)hash_panid;
  zb_address_pan_id_ref_t i;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id_ref pan_id " TRACE_FORMAT_64 " ref %p", (FMT__A_P, TRACE_ARG_64(pan_id), ref));

  /* try to find pan id */
  TRACE_MSG(TRACE_COMMON3, "start search pan_id reference from h_i %hd", (FMT__H, h_i));
  i = h_i;
  do
  {
    if (zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, i)
         && ZB_EXTPANID_CMP(ZG->addr.pan_map[i].long_panid, pan_id) )
    {
      TRACE_MSG(TRACE_COMMON1, "pan_id reference found %d", (FMT__D, (int)i));
      ret = RET_OK;
      *ref = i;
      break;
    }
    /* ZB_PANID_TABLE_SIZE is power of 2, so it is cheap to use division */
    /* i = ( i + 1 ) % ZB_PANID_TABLE_SIZE;
       if use sdcc, then code size increases by ~45 bytes  :)
    */
    i = i + 1U;
    i = i % ZB_PANID_TABLE_SIZE;
  } while ( i != h_i );

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id_ref %d", (FMT__D, ret));
  return ret;
}


void zb_address_get_short_pan_id(zb_address_pan_id_ref_t pan_id_ref, zb_uint16_t *pan_id_p)
{
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_get_pan_id pan_id_reference %d pan_id %p", (FMT__D_P, (int)pan_id_ref, pan_id_p));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) == ZB_TRUE );
  *pan_id_p = ZG->addr.pan_map[pan_id_ref].short_panid;

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_get_pan_id", (FMT__0));
}


zb_bool_t zb_address_cmp_pan_id_by_ref(zb_address_pan_id_ref_t pan_id_ref, zb_ext_pan_id_t pan_id)
{
  zb_bool_t ret;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_cmp_pan_id_by_ref pan_id_reference %d pan_id %p", (FMT__D_P,(int) pan_id_ref, pan_id));

  ZB_ASSERT( pan_id_ref < ZB_PANID_TABLE_SIZE
             && zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, pan_id_ref) == ZB_TRUE );
  ret = ZB_EXTPANID_CMP(pan_id, ZG->addr.pan_map[pan_id_ref].long_panid);

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_cmp_pan_id_by_ref %d", (FMT__D, ret));
  return ret;
}

zb_ret_t zb_address_reuse_pan_id(zb_uint16_t short_pan_id, zb_ext_pan_id_t pan_id, zb_address_pan_id_ref_t panid_ref)
{
  zb_ret_t ret = RET_NOT_FOUND;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_reuse_pan_id %hd", (FMT__H, panid_ref));

  if (zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, panid_ref))
  {
    TRACE_MSG(TRACE_COMMON2, "Replace old short_pan_id %d pan_id " TRACE_FORMAT_64 "  with new short_pan_id %d pan_id " TRACE_FORMAT_64,
              (FMT__D_A_D_A, ZG->addr.pan_map[panid_ref].short_panid, TRACE_ARG_64(ZG->addr.pan_map[panid_ref].long_panid),
               short_pan_id, TRACE_ARG_64(pan_id)));

    ret = RET_OK;
    ZB_EXTPANID_COPY(ZG->addr.pan_map[panid_ref].long_panid, pan_id);
    ZG->addr.pan_map[panid_ref].short_panid = short_pan_id;
  }

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_reuse_pan_id %d", (FMT__D, ret));

  return ret;
}

zb_ret_t zb_address_delete_pan_id(zb_address_pan_id_ref_t panid_ref)
{
  zb_ret_t ret = RET_NOT_FOUND;

  TRACE_MSG(TRACE_COMMON1, ">>zb_address_delete_pan_id %d", (FMT__H, panid_ref));

  if (zb_check_bit_in_bit_vector(ZG->addr.used_pan_addr, panid_ref))
  {
    zb_clr_bit_in_bit_vector(ZG->addr.used_pan_addr, panid_ref);
    ret = RET_OK;
  }

  TRACE_MSG(TRACE_COMMON1, "<<zb_address_delete_pan_id %d", (FMT__D, ret));

  return ret;
}


static zb_ret_t get_manufacturer_index_by_ieee_addr(const zb_ieee_addr_t address, zb_ushort_t *idx)
{
  zb_ushort_t i;
  zb_ushort_t tbl_idx = ZB_DEV_MANUFACTURER_TABLE_SIZE;
  zb_ret_t ret = RET_OK;

  for ( i = 0; i < ZB_DEV_MANUFACTURER_TABLE_SIZE; i++)
  {
    if (zb_check_bit_in_bit_vector(ZG->addr.used_manufacturer, i))
    {
      /*
        * AS: Fixed wrong division 64-bit extended address into
        * manufacturer specific and device unique parts.
      */
      if (ZB_MEMCMP(&ZG->addr.dev_manufacturer[i].device_manufacturer[0], &address[5], 3) == 0)
      {
        tbl_idx = i;
        break;
      }
    }
    else
    {
      tbl_idx = i;
    }
  }

  if (i == ZB_DEV_MANUFACTURER_TABLE_SIZE && tbl_idx == ZB_DEV_MANUFACTURER_TABLE_SIZE)
  {
    ret = RET_NO_MEMORY;
  }
  else
  {
    *idx = tbl_idx;
  }

  return ret;
}


static void manuf_tbl_clear_unusable_entries(void)
{
  zb_address_map_t *ent;
  zb_ushort_t i;

  TRACE_MSG(TRACE_COMMON4, "manuf_tbl_clear_unusable_entries", (FMT__0));

  /* Reset manufacturer bitmask */
  ZB_BZERO(ZG->addr.used_manufacturer, ZB_DEV_MANUFACTURER_TABLE_SIZE / 8U + 1U);

  /* Regenerate manufacturer bitmask from address table */
  for ( i = 0; i < ZB_IEEE_ADDR_TABLE_SIZE; i++)
  {
    ent = &ZG->addr.addr_map[i];
    if (ZB_U2B(ent->used) && ent->redirect_type == ZB_ADDR_REDIRECT_NONE)
    {
      zb_set_bit_in_bit_vector(ZG->addr.used_manufacturer, ent->ieee_addr.dev_manufacturer);
    }
  }
}


zb_bool_t zb_address_check_mem_for_new_addr(const zb_ieee_addr_t new_addr)
{
  zb_ushort_t i;
  zb_bool_t ret = ZB_TRUE;

  if (RET_NO_MEMORY == get_manufacturer_index_by_ieee_addr(new_addr, &i))
  {
    manuf_tbl_clear_unusable_entries();
    ret = (RET_OK == get_manufacturer_index_by_ieee_addr(new_addr, &i));
  }

  if (ret == RET_OK)
  {
    zb_address_ieee_ref_t ref;

    if (RET_OK != zb_address_by_ieee(new_addr, ZB_FALSE, ZB_FALSE, &ref))
    {
      ret = ZB_FALSE;
      for ( i = 0; i < ZB_IEEE_ADDR_TABLE_SIZE; i++)
      {
        if (!ZB_U2B(ZG->addr.addr_map[i].used))
        {
          ret = ZB_TRUE;
          break;
        }
      }
    }
  }

  return ret;
}


void zb_ieee_addr_compress(const zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address)
{
  zb_ushort_t i;
  zb_ret_t ret;

  /* try to find this manufacturer */
  ret = get_manufacturer_index_by_ieee_addr(address, &i);

  /* if manufacturer table is full, try to clear unusable entries */
  if (ret == RET_NO_MEMORY)
  {
    manuf_tbl_clear_unusable_entries();
    ret = get_manufacturer_index_by_ieee_addr(address, &i);
  }

  /* In normal situation we shouldn't reach this assertion.
   * Because we must handle the OOM situation earlier using zb_address_check_mem_for_new_addr()*/
  ZB_ASSERT(ret == RET_OK);

  /* compress */
  zb_set_bit_in_bit_vector(ZG->addr.used_manufacturer, i);
  ZB_MEMCPY(&ZG->addr.dev_manufacturer[i].device_manufacturer[0], &address[5], 3);

  compressed_address->dev_manufacturer = (zb_uint8_t)i;
  ZB_MEMCPY(&compressed_address->device_id[0], &address[0], 5);
}


void zb_ieee_addr_decompress(zb_ieee_addr_t address, zb_ieee_addr_compressed_t *compressed_address)
{
  ZB_ADDRESS_DECOMPRESS(address, *compressed_address);
}


/**
   Calculate hash function for the compressed long address
 */
#define ZB_ADDRESS_COMPRESSED_IEEE_HASH_MACRO(addr) ( ZB_4INT_HASH_FUNC(  \
  (zb_uint16_t)(addr)->device_id[0],                                \
  (zb_uint16_t)(addr)->device_id[2],                                \
  (zb_uint16_t)(addr)->device_id[3],                                \
  (zb_uint16_t)(addr)->device_id[4]) % ZB_IEEE_ADDR_TABLE_SIZE )


static zb_address_ieee_ref_t zb_address_compressed_ieee_hash(zb_ieee_addr_compressed_t *ieee_compressed)
{
  return (zb_address_ieee_ref_t)ZB_ADDRESS_COMPRESSED_IEEE_HASH_MACRO(ieee_compressed);
}

zb_ret_t zb_address_by_ieee(const zb_ieee_addr_t ieee, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p)
{
  zb_ret_t ret = RET_OK;
  zb_ieee_addr_compressed_t ieee_compressed;
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_ieee ieee %p lock %hd", (FMT__P_H, ieee, lock));

  ZB_DUMP_IEEE_ADDR(ieee);
  /* compress address */
  zb_ieee_addr_compress(ieee, &ieee_compressed);
  /* search */
  if (ieee_search(&ieee_compressed, ref_p))
  {
    /* mark address as updated - prevent its immediate reuse */
    ZG->addr.addr_map[*ref_p].clock = 1U;
  }
  else
  {
    ret = ( create ) ? addr_add(&ieee_compressed, (zb_uint16_t)(~0U), ref_p) : RET_NOT_FOUND;
  }

  if (lock && ret == RET_OK)
  {
    ret = zb_address_lock(*ref_p);
  }
  TRACE_MSG(TRACE_COMMON3, "<< zb_address_ieee, ret %i", (FMT__D, ret));
  return ret;
}


zb_ret_t zb_address_by_short(zb_uint16_t short_address, zb_bool_t create, zb_bool_t lock, zb_address_ieee_ref_t *ref_p)
{
  zb_ret_t ret = RET_OK;
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_by_short addr 0x%x lock %hd", (FMT__D_H, short_address, lock));

  if (short_search(short_address, ref_p))
  {
    /* mark address as updated - prevent its immediate reuse */
    ZG->addr.addr_map[*ref_p].clock = 1U;
  }
  else
  {
    ret = ( create ) ? addr_add(NULL, short_address, ref_p) : RET_NOT_FOUND;
  }
  if (lock && ret == RET_OK)
  {
    ret = zb_address_lock(*ref_p);
  }
  if (ret == RET_OK)
  {
    TRACE_MSG(TRACE_COMMON3, "<<zb_address_by_short 0x%x ret %hd ref %hd cnt %hd", (FMT__D_H_H_H, short_address, ret, *ref_p, ZG->addr.addr_map[*ref_p].lock_cnt));
  }
  else
  {
    TRACE_MSG(TRACE_COMMON3, "<<zb_address_by_short 0x%x ret %hd ref Unknown", (FMT__D_H, short_address, ret));
  }
  return ret;
}

zb_ret_t zb_address_by_sorted_table_index(zb_ushort_t index, zb_address_ieee_ref_t *ref_p)
{
  if(index < ZG->addr.n_sorted_elements)
  {
    *ref_p = ZG->addr.short_sorted[index];
    return RET_OK;
  }
  return RET_NOT_FOUND;
}

void zb_long_address_update_by_ref(zb_ieee_addr_t ieee_address, zb_address_ieee_ref_t ref)
{
  zb_address_map_t *ent;
  zb_ieee_addr_compressed_t ieee_compressed;

  ent = &ZG->addr.addr_map[ref];

  if (ent->redirect_type != ZB_ADDR_REDIRECT_NONE)
  {
    ent = &ZG->addr.addr_map[ent->redirect_ref];
  }

  zb_ieee_addr_compress(ieee_address, &ieee_compressed);
  ent->ieee_addr = ieee_compressed;
}

zb_ret_t zb_address_update(zb_ieee_addr_t ieee_address, zb_uint16_t short_address, zb_bool_t lock, zb_address_ieee_ref_t *ref_p)
{
  zb_ret_t ret = RET_OK;
  zb_bool_t found_short;
  zb_bool_t found_long;
  zb_address_map_t *ent;
  zb_address_ieee_ref_t ref;
  zb_address_ieee_ref_t ref_long = 0;
  zb_ieee_addr_compressed_t ieee_compressed;

#ifdef ZB_TH_ENABLED
  mac_sync_address_update(ieee_address, short_address, lock);
#endif

  /* compress address */
  TRACE_MSG(TRACE_COMMON3,
      "> zb_address_update addr " TRACE_FORMAT_64 " short_address 0x%x lock %i",
      (FMT__A_D_D, TRACE_ARG_64(ieee_address), short_address, lock));
  TRACE_MSG(TRACE_COMMON3, "ref_p %p", (FMT__P, ref_p));
  found_short = short_search(short_address, &ref);
  if (!ZB_IEEE_ADDR_IS_UNKNOWN(ieee_address))
  {
    zb_ieee_addr_compress(ieee_address, &ieee_compressed);
    found_long = ieee_search(&ieee_compressed, &ref_long);
  }
  else
  {
    ZB_ADDRESS_COMPRESS_UNKNOWN(ieee_compressed);
    found_long = ZB_FALSE;
  }

  if (found_short && found_long)
  {
    TRACE_MSG(TRACE_COMMON3, "found long %hd & short %hd", (FMT__H_H, ref_long, ref));
    if (ref != ref_long)
    {
      if ( !ZB_U2B(ZG->addr.addr_map[ref].lock_cnt) )
      {
        ret = zb_address_delete(ref);
        ref = ref_long;
      }
      else if ( !ZB_U2B(ZG->addr.addr_map[ref_long].lock_cnt) )
      {
        ret = zb_address_delete(ref_long);
      }
      else
      {
#ifndef ZB_MAC_ONLY_STACK
        TRACE_MSG(TRACE_COMMON3, "short ref %hd lock_cnt %hd, ref_long %hd lock_cnt %hd",
                  (FMT__H_H_H_H, ref, ZG->addr.addr_map[ref].lock_cnt, ref_long, ZG->addr.addr_map[ref_long].lock_cnt));
        /*
          Resolve conflict: 2 locked entries with long/short address mismatch.
          Not sure how to do it. Scan for _every_ data structure which use
          address and remove it??
        */
        /* The idea is to redirect the record with the smaller
         * lock counter to the another record. Unlock and kill the
         * redirected entry as soon as possible */

        if (ZG->addr.addr_map[ref].lock_cnt > ZG->addr.addr_map[ref_long].lock_cnt
            /* Prefer the entry with nbt if another entry does not have nbt */
            /*cstat -MISRAC2012-Rule-13.5 */
            /* After some investigation, the following violation of Rule 13.5 seems to be
             * a false positive. There are no side effect to 'zb_aib_trust_center_address_cmp()'.
             * This violation seems to be caused by the fact that
             * 'zb_aib_trust_center_address_cmp()' is an
             * external function, which cannot be analyzed by C-STAT. */
            || (zb_nwk_neighbor_exists(ref) && !zb_nwk_neighbor_exists(ref_long)))
            /*cstat +MISRAC2012-Rule-13.5 */
        {
          /* Record with IEEE addr will be redirect record */
          TRACE_MSG(TRACE_COMMON3, "Redirect entry %hd to %hd, REDIRECT_IEEE", (FMT__H_H, ref_long, ref));

          ZG->addr.addr_map[ref].lock_cnt += ZG->addr.addr_map[ref_long].lock_cnt;
          ZG->addr.addr_map[ref].redirect_ref = ref_long;

          ent = &ZG->addr.addr_map[ref_long];
          ent->redirect_type = ZB_ADDR_REDIRECT_IEEE;
          ent->redirect_ref = ref;

          ZB_ADDRESS_COMPRESSED_COPY(ent->ieee_addr, ieee_compressed);
        }
        else
        {
          /* Record with short addr will be redirect record */
          TRACE_MSG(TRACE_COMMON3, "Redirect entry %hd to %hd, REDIRECT_SHORT", (FMT__H_H, ref, ref_long));

          ZG->addr.addr_map[ref_long].lock_cnt += ZG->addr.addr_map[ref].lock_cnt;
          ZG->addr.addr_map[ref_long].redirect_ref = ref;

          ent = &ZG->addr.addr_map[ref];
          ent->redirect_type = ZB_ADDR_REDIRECT_SHORT;
          ent->redirect_ref = ref_long;
          ent->addr = short_address;

          /* Delete this address from short sorted - keep it using
             references to only regular records. Anyway, the short
             address will be stored in the sorted array: see the code
             below */
          del_short_sorted(ref);

          /* Will update the regular record containing the IEEE
           * address, see the code below */
          ref = ref_long;
        }
#endif  /* ZB_MAC_ONLY_STACK */
      }

      if (ret != RET_OK)
      {
        ZB_ASSERT(0);
        return ret;
      }
    }
  }
  else if (found_long)
  {
    TRACE_MSG(TRACE_COMMON3, "found long", (FMT__0));
    ref = ref_long;
  }
  else
  {
    /* MISRA rule 15.7 requires empty 'else' branch. */
    /* If short or long address is found 'ref' is set .
     * If no address was found, normally continue
     * execution with 'ref' uninitialized. */
  }

  if (found_short || found_long)
  {
    TRACE_MSG(TRACE_COMMON3, "found long || short", (FMT__0));
    ent = &ZG->addr.addr_map[ref];

    if ((short_address == 0x0000U) && (ent->addr == 0x0000U)
        && !ZB_ADDRESS_COMPRESSED_CMP(ent->ieee_addr, ieee_compressed)
        && !ZB_ADDRESS_COMPRESSED_IS_UNKNOWN(ent->ieee_addr)
        /* 09/21/2016 EE CR:MINOR Is it ever possible to have zero long address?*/
        && !ZB_ADDRESS_COMPRESSED_IS_ZERO(ent->ieee_addr)
        && (ZB_U2B(ZG->addr.addr_map[ref].lock_cnt)))
    {
      /* We have already ZC long + short addresses - do nothing:
         prevent ZC long address change upon PAN ID conflict (see TP-PRO-BV-29[30]) */
      *ref_p = ref;
      ret = RET_ALREADY_EXISTS;
    }
    else
    {
      /* TODO: if address has changed and not locked, try to relocate it for better
       * long address hash performance */
      ZB_MEMCPY(&ent->ieee_addr, &ieee_compressed, sizeof(ieee_compressed));
      if (ent->addr != short_address)
      {
        if (ent->addr != (zb_uint16_t)~0U)
        {
          del_short_sorted(ref);
        }
        ent->addr = short_address;
        zb_add_short_addr_sorted(ref, short_address);
      }
      ent->clock = 1U;
    }
  }
  else
  {
    ret = addr_add(&ieee_compressed, short_address, &ref);
  }
  if (ret == RET_OK)
  {
    *ref_p = ref;

    if (lock)
    {
      ret = zb_address_lock(ref);
    }

    /* Address was updated - reset pending_for_delete flag.

     * pending_for_delete is necessary for some complex cases - for example when we have pending
     * data_request (device is offline), and ed aging kills the record before confirm. In such case
     * we make address as pending for delete but actually delete it only when it is unlocked. If
     * this record was updated before last unlock (when it already pending for delete) - it is
     * actually used again, and we will not delete it. */
    if (ZB_U2B(ZG->addr.addr_map[ref].pending_for_delete))
    {
      TRACE_MSG(TRACE_ERROR, "WARNING: clear pending_for_delete for ref %hd", (FMT__H, ref));
    }
    ZG->addr.addr_map[ref].pending_for_delete = ZB_FALSE_U;
  }
  TRACE_MSG(TRACE_COMMON3, "< zb_address_update short 0x%x long " TRACE_FORMAT_64,
            (FMT__D_A, short_address, TRACE_ARG_64(ieee_address)));
  TRACE_MSG(TRACE_COMMON3, "< zb_address_update (part 2) ref %d cnt %d ret %d", (FMT__D_D_D, ref, ZG->addr.addr_map[ref].lock_cnt, (zb_uint8_t)ret));
  return ret;
}


void zb_address_by_ref(zb_ieee_addr_t ieee_address, zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref)
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];

  if (ent->redirect_type != ZB_ADDR_REDIRECT_NONE)
  {
    ent = &ZG->addr.addr_map[ent->redirect_ref];
  }

  *short_address_p = ent->addr;
  ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);
}


void zb_address_ieee_by_ref(zb_ieee_addr_t ieee_address, zb_address_ieee_ref_t ref)
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];

  if (ent->redirect_type != ZB_ADDR_REDIRECT_NONE)
  {
    ent = &ZG->addr.addr_map[ent->redirect_ref];
  }

  if (!ZB_ADDRESS_COMPRESSED_IS_ZERO(ent->ieee_addr))
  {
    ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);
  }
  else
  {
    ZB_64BIT_ADDR_ZERO(ieee_address);
  }
}


void zb_address_short_by_ref(zb_uint16_t *short_address_p, zb_address_ieee_ref_t ref)
{
  zb_address_map_t *ent = &ZG->addr.addr_map[ref];

  if (ent->redirect_type != ZB_ADDR_REDIRECT_NONE)
  {
    ent = &ZG->addr.addr_map[ent->redirect_ref];
  }

  ZB_MEMCPY(short_address_p, &ent->addr, sizeof(zb_uint16_t));
}


zb_uint16_t zb_address_short_by_ieee(zb_ieee_addr_t ieee_address)
{
  zb_address_ieee_ref_t ref;
  if (zb_address_by_ieee(ieee_address, ZB_FALSE, ZB_FALSE, &ref) == RET_OK)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[ref];
    return ent->addr;
  }
  return ZB_UNKNOWN_SHORT_ADDR;
}


zb_ret_t zb_address_ieee_by_short(zb_uint16_t short_addr, zb_ieee_addr_t ieee_address)
{
  zb_ret_t ret = RET_NOT_FOUND;
  zb_address_ieee_ref_t ref;

  if (zb_address_by_short(short_addr, ZB_FALSE, ZB_FALSE, &ref) == RET_OK)
  {
    zb_address_map_t *ent = &ZG->addr.addr_map[ref];
    ZB_ADDRESS_DECOMPRESS(ieee_address, ent->ieee_addr);

    /* nwk_add_into_addr_and_nbt() can add only short address.
     * Decompress returns unknown address in this case. */
    if (ZB_IEEE_ADDR_IS_VALID(ieee_address))
    {
      ret = RET_OK;
    }
  }
  return ret;
}


zb_bool_t zb_address_is_locked(zb_address_ieee_ref_t ref)
{
  ZB_ASSERT(ZG->addr.addr_map[ref].used);
  return (zb_bool_t)(ZG->addr.addr_map[ref].lock_cnt > 0U);
}


zb_ret_t zb_address_lock_func(TRACE_ADDR_PROTO zb_address_ieee_ref_t ref)
{
#ifdef ZB_DEBUG_ADDR
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_lock ref %hd cnt %hd from ZB_TRACE_FILE_ID %d:%d",
            (FMT__H_H_D_D, ref, ZG->addr.addr_map[ref].lock_cnt+1, TRACE_ADDR_FORWARD));
#else
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_lock ref %hd cnt %hd",
            (FMT__H_H, ref, ZG->addr.addr_map[ref].lock_cnt+1));
#endif

  if (!ZB_U2B(ZG->addr.addr_map[ref].used))
  {
#ifdef ZB_DEBUG_ADDR
    TRACE_MSG(TRACE_ERROR, "zb_address_lock from ZB_TRACE_FILE_ID %d:%d error: entry is not used",
              (FMT__D_D, TRACE_ADDR_FORWARD));
#else
    TRACE_MSG(TRACE_ERROR, "zb_address_lock error: entry is not used", (FMT__0));
#endif
    return RET_ERROR;
  }

  ZG->addr.addr_map[ref].lock_cnt++;
  ZB_ASSERT(ZG->addr.addr_map[ref].lock_cnt != 0U); /* check for overflow */
  ZG->addr.addr_map[ref].clock = 1U;

  if (ZG->addr.addr_map[ref].redirect_type != ZB_ADDR_REDIRECT_NONE)
  {
    zb_address_ieee_ref_t regular_ref;

    regular_ref = ZG->addr.addr_map[ref].redirect_ref;

    /* Update regular record */
    ZG->addr.addr_map[regular_ref].lock_cnt++;
    ZB_ASSERT(ZG->addr.addr_map[regular_ref].lock_cnt != 0U); /* check for overflow */
    ZG->addr.addr_map[regular_ref].clock = 1U;
  }

  TRACE_MSG(TRACE_COMMON3, "<<zb_address_lock", (FMT__0));
  return RET_OK;
}

static void zb_address_unlock_assert_locked(TRACE_ADDR_PROTO zb_address_ieee_ref_t ref)
{
  if (ZG->addr.addr_map[ref].lock_cnt == 0U)
  {
    zb_uint16_t short_addr;
    zb_ieee_addr_t ieee_addr;
    zb_address_map_t *ent;

    ent = &ZG->addr.addr_map[ref];
    zb_address_short_by_ref(&short_addr, ref);
    zb_address_ieee_by_ref(ieee_addr, ref);

#if !TRACE_ENABLED(TRACE_ERROR)
    ZVUNUSED(ent);
#endif

#ifdef ZB_DEBUG_ADDR
    TRACE_MSG(TRACE_ERROR, "zb_address_unlock from %d:%d error: ref %d not locked", (FMT__D_D_D, TRACE_ADDR_FORWARD, ref));
#else
    TRACE_MSG(TRACE_ERROR, "zb_address_unlock error: ref %d not locked", (FMT__D, ref));
#endif

    TRACE_MSG(TRACE_ERROR, "has_address_conflict %hu", (FMT__H, ent->has_address_conflict));
    TRACE_MSG(TRACE_ERROR, "lock_cnt %hu, pending_for_delete %hu", (FMT__H_H, ent->lock_cnt, ent->pending_for_delete));
    TRACE_MSG(TRACE_ERROR, "clock %hu", (FMT__H, ent->clock));
    TRACE_MSG(TRACE_ERROR, "redirect_type %hu, redirect_ref %hu", (FMT__H_H, ent->redirect_type, ent->redirect_ref));
    TRACE_MSG(TRACE_ERROR, "short 0x%x, ieee " TRACE_FORMAT_64, (FMT__H_A, short_addr, TRACE_ARG_64(ieee_addr)));

    ZB_ASSERT(0);
  }
}

void zb_address_unlock_func(TRACE_ADDR_PROTO zb_address_ieee_ref_t ref)
{
#ifdef ZB_DEBUG_ADDR
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_unlock from ZB_TRACE_FILE_ID %d:%d ref %hd, cnt %hd new cnt %hd redirect %hd",
            (FMT__D_D_H_H_H_H, TRACE_ADDR_FORWARD,
#else
  TRACE_MSG(TRACE_COMMON3, ">>zb_address_unlock ref %hd, cnt %hd new cnt %hd redirect %hd",
            (FMT__H_H_H_H,
#endif
             ref,
             ZG->addr.addr_map[ref].lock_cnt,
             ZG->addr.addr_map[ref].lock_cnt-1,
             ZG->addr.addr_map[ref].redirect_type));

  if (!ZB_U2B(ZG->addr.addr_map[ref].used))
  {
#ifdef ZB_DEBUG_ADDR
    TRACE_MSG(TRACE_ERROR, "zb_address_unlock from ZB_TRACE_FILE_ID %d:%d error: entry is not used", (FMT__D_D, TRACE_ADDR_FORWARD));
#else
    TRACE_MSG(TRACE_ERROR, "zb_address_unlock error: entry is not used", (FMT__0));
#endif
    /* Invalid address, abort the execution */
    ZB_ASSERT(ZB_FALSE);
    return;
  }

  if (ZG->addr.addr_map[ref].redirect_type == ZB_ADDR_REDIRECT_NONE)
  {
    /* Unlock the regular record: decrement only regular lock counter */
    zb_address_ieee_ref_t redirect_ref;

    redirect_ref = ZG->addr.addr_map[ref].redirect_ref;
    if (redirect_ref != (zb_address_ieee_ref_t)(-1))
    {
      /* Ensure that regular record contains more locks than redirect record */
      /* ZB_ASSERT(ZG->addr.addr_map[ref].lock_cnt > ZG->addr.addr_map[redirect_ref].lock_cnt); */
      /* fix assert: we have regular record here, so it can refer to garbage in redirect_ref */
      ZG->addr.addr_map[ref].redirect_ref = (zb_address_ieee_ref_t)(-1);
    }

    zb_address_unlock_assert_locked(TRACE_ADDR_FORWARD ref);
    ZG->addr.addr_map[ref].lock_cnt--;
  }
  else
  {
    /* Unlock the redirect record: decrement both - redirect and regular */
    zb_address_ieee_ref_t regular_ref;

    regular_ref = ZG->addr.addr_map[ref].redirect_ref;

    zb_address_unlock_assert_locked(TRACE_ADDR_FORWARD ref);
    ZG->addr.addr_map[ref].lock_cnt--;

    zb_address_unlock_assert_locked(TRACE_ADDR_FORWARD regular_ref);
    ZG->addr.addr_map[regular_ref].lock_cnt--;

    if (ZG->addr.addr_map[ref].lock_cnt == 0U)
    {
      zb_ret_t ret;

      ret = zb_address_delete(ref);
      ZB_ASSERT(ret == RET_OK);
      ZG->addr.addr_map[regular_ref].redirect_ref = (zb_address_ieee_ref_t)(-1);
    }
  }

  /* NK: Now we kill address map entry only when delete was called (if no locks - immediate delete, else
   * pending_for_delete == true), it is no locks and record was not updated (update resets
   * pending_for_delete flag). */
  /* If address was marked as "pending for delete" before and it is no more locks, remove it. */
  if ((ZG->addr.addr_map[ref].lock_cnt == 0U) && ZB_U2B(ZG->addr.addr_map[ref].pending_for_delete))
  {
    zb_ret_t ret;

    TRACE_MSG(TRACE_ERROR, "WARNING: remove ref %hd which was in pending_for_delete", (FMT__H, ref));
    ret = zb_address_delete(ref);
    ZB_ASSERT(ret == RET_OK);
  }
  TRACE_MSG(TRACE_COMMON3, "<<zb_address_unlock", (FMT__0));
}


/**
   Search address by 64-bit address

   Use open hash search. In the worst case it works like linear search.

   @param ieee_compressed - compressed long address
   @param ref_p - (out) address reference

   @return ZB_TRUE if found, else ZB_FALSE
 */
static zb_bool_t ieee_search(zb_ieee_addr_compressed_t *ieee_compressed, zb_address_ieee_ref_t *ref_p)
{
  zb_address_ieee_ref_t i = zb_address_compressed_ieee_hash(ieee_compressed);
  zb_address_ieee_ref_t cnt;
  zb_address_map_t *ent;

  for (cnt = 0 ; cnt < ZB_IEEE_ADDR_TABLE_SIZE ; ++cnt)
  {
    ent = &ZG->addr.addr_map[i];

    if ( ZB_U2B(ent->used) &&
         /* Search in regular entries... */
         ((ent->redirect_type == ZB_ADDR_REDIRECT_NONE &&
           ZB_ADDRESS_COMPRESSED_CMP(ent->ieee_addr, *ieee_compressed)) ||
          /* ... or redirect entries */
          (ent->redirect_type == ZB_ADDR_REDIRECT_IEEE &&
           ZB_ADDRESS_COMPRESSED_CMP(ent->ieee_addr, *ieee_compressed))))
    {
      TRACE_MSG(TRACE_COMMON3, "ieee reference found %d", (FMT__D, (int)i));
      *ref_p = (ent->redirect_type == ZB_ADDR_REDIRECT_NONE) ? i : ent->redirect_ref;
      return ZB_TRUE;
    }
    i = (i + 1U) % ZB_IEEE_ADDR_TABLE_SIZE;
  }

  return ZB_FALSE;
}


/**
   Search address by 16-bit address

   Use binary search over short_sorted array.

   @param short_addr - short address
   @param ref_p - (out) address reference

   @return ZB_TRUE if found, else ZB_FALSE
 */
static zb_bool_t short_search(zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p)
{
  zb_bool_t found = ZB_FALSE;
  zb_ushort_t l = 0;

  /* when using signed bytes as indexes, table size must be < 127 */
#ifndef ZB_CONFIGURABLE_MEM
  ZB_ASSERT_COMPILE_DECL(ZB_IEEE_ADDR_TABLE_SIZE <= (zb_ushort_t)ZB_SHORT_MAX);
#endif

  /* if has exactly 1 element, not need to do binary search */
  if (ZG->addr.n_sorted_elements > 1U)
  {
    /* search using binary search */
    zb_ushort_t r = ZG->addr.n_sorted_elements;
    while (l < r)
    {
      zb_ushort_t i = l + (r - l) / 2U;
      if (short_addr > ZG->addr.addr_map[ZG->addr.short_sorted[i]].addr)
      {
        l = i + 1U;
      }
      else
      {
        r = i;
      }
    }
  }

  TRACE_MSG(TRACE_COMMON3, "short_addr 0x%x l %d ZG->addr.short_sorted[l] %d addr 0x%x",
            (FMT__D_D_D_D, short_addr, l, ZG->addr.short_sorted[l], ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr));

  if (/* Strict condition for prevention array borders violation */
      l < ZB_IEEE_ADDR_TABLE_SIZE
      && ZG->addr.n_sorted_elements > 0U   /* empty? */
      && l < ZG->addr.n_sorted_elements
      && short_addr == ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr)
  {
    *ref_p = ZG->addr.short_sorted[l];
    found = ZB_TRUE;
  }

  return found;
}


/**
   Add ieee/short address pair.
   Allocate space, add an entry.

   Possible problem: entry can't be moved once allocated.
   If entry first creates without long address, its search will be not
   effective!

   @param ieee_compressed - pointer to the compressed long address, NULL if absent
   @param short_addr - short address, 0 if absent

 */
static zb_ret_t addr_add(zb_ieee_addr_compressed_t *ieee_compressed,
                         zb_uint16_t short_addr, zb_address_ieee_ref_t *ref_p)
{
  zb_address_ieee_ref_t i = (ieee_compressed != NULL) ? zb_address_compressed_ieee_hash(ieee_compressed) : 0U;
  zb_address_ieee_ref_t i_end = i;
  zb_address_map_t *ent = NULL;
  zb_uint8_t pass;

  TRACE_MSG(TRACE_COMMON3, "addr_add 0x%x", (FMT__D, short_addr));

  /* First find a place to insert */
  for (pass = 0U ; pass < 3U ; pass += ZB_B2U(i == i_end))
  {
    ent = &ZG->addr.addr_map[i];
    /* Old comment: Correct enries reuse. Seems it should reuse items only when it
     * doesn't have room for new entry */
    /* EE: Check for lock_cnt was commented out from the prehistoric time,
     * so entire idea of locks/caching did not work risking to translation
     * table overflow. Now fixing it, but it is risky if some not locked
     * address is in use somewhere.
     *
     * Now first search for not used entry, next try to find not locked one with
     * clock == 0, next try to find any not locked entry.
     */
    if (!ZB_U2B(ent->used) || (ZB_U2B(pass) && ent->lock_cnt == 0U && (pass == 2U || !ZB_U2B(ent->clock))))
    {
      break;
    }
    i = (i + 1U) % ZB_IEEE_ADDR_TABLE_SIZE;
  }

  if (pass == 3U)
  {
    TRACE_MSG(TRACE_ERROR, "Achtung! Address translation table overflow!", (FMT__0));
    return RET_OVERFLOW;
  }

  /* expire some cached (not locked) entry. Use clock algorithm to keep
   * recently used entries. */
  clock_tick();

  ent->addr = short_addr;
  ent->clock = 1U;
  ZB_ASSERT(ent->lock_cnt == 0U);            /* It must be already 0 (zeroed
                                             * during init or in zb_address_delete) */
  if (ieee_compressed != NULL)
  {
    ZB_MEMCPY(&ent->ieee_addr, ieee_compressed, sizeof(*ieee_compressed));
  }
  else
  {
    ZB_ADDRESS_COMPRESS_UNKNOWN(ent->ieee_addr);
  }
  /* Now update short sorted array if short address exists */
  if (short_addr != (zb_uint16_t)(~0U))
  {
    if (ZB_U2B(ent->used))
    {
      del_short_sorted(i);
    }
    zb_add_short_addr_sorted(i, short_addr);
  }
  ZG->addr.n_elements += ZB_B2U(!ZB_U2B(ent->used));
  ent->redirect_type = ZB_ADDR_REDIRECT_NONE;
  ent->redirect_ref = (zb_address_ieee_ref_t)(-1);
  ent->used = ZB_TRUE_U;
  ent->has_address_conflict = ZB_FALSE_U;
  *ref_p = i;

  TRACE_MSG(TRACE_COMMON3, "added to ref %hd lock_cnt %hd n_sorted_elements %hd", (FMT__H_H_H, i, ZG->addr.addr_map[i].lock_cnt, ZG->addr.n_sorted_elements));

  return RET_OK;
}


/**
   Delete address from the translation array

   @param ref - address ref
 */
zb_ret_t zb_address_delete(zb_address_ieee_ref_t ref)
{
  TRACE_MSG(TRACE_ERROR, "zb_address_delete ref %hd", (FMT__H, ref));

  /* This assert is VERY useful for addr_locks mismatches debug - we really have too few cases when
   * lock_cnt is not 0 and delete really should be delayed (at the moment only one case - device was
   * aged when pending apsde_data_req is in progress). */

  if (!ZB_U2B(ZG->addr.addr_map[ref].used))
  {
    TRACE_MSG(TRACE_ERROR, "zb_address_delete error: entry is not used", (FMT__0));
    return RET_ERROR;
  }

  /* If address is not locked, delete immediately, else mark this address as "pending for delete" -
   * if it will be no update operation on this addr, at the moment of (lock cnt == 0) it will be deleted. */
  /* WARNING: Locked address should not be removed anywhere! */
  if (ZG->addr.addr_map[ref].lock_cnt == 0U)
  {
    del_short_sorted(ref);
    ZG->addr.addr_map[ref].used = ZB_FALSE_U;
    /* Clear lock_cnt and clock on addr delete */
    ZG->addr.addr_map[ref].lock_cnt = 0U;
    ZG->addr.addr_map[ref].clock = 0U;
    ZG->addr.n_elements--;
  }
  else
  {
    /* NK: If it was no locks, LEAVE will remove the addr (as previous). If
     * there was locks, LEAVE just marks map entry for delete. It is up to the
     * upper level code to unlock address properly by removing all references to
     * it. */
    TRACE_MSG(TRACE_ERROR, "WARNING: pending_for_delete ref %hd", (FMT__H, ref));
    ZG->addr.addr_map[ref].pending_for_delete = ZB_TRUE_U;
  }
  return RET_OK;
}


/**
   Delete en entry from the short address sorted search array

   @param ref - address ref
 */
static void del_short_sorted(zb_address_ieee_ref_t ref)
{
  zb_address_ieee_ref_t i;
  zb_ushort_t dec = 0;
  TRACE_MSG(TRACE_COMMON3, ">>del_short_sorted ref %d", (FMT__D, (int)ref));
  /* Use trivial scan because dups in the short address are possible */
  for (i = 0 ; i < ZG->addr.n_sorted_elements ; ++i)
  {
    if (ZG->addr.short_sorted[i] == ref)
    {
      dec++;
      if (i != ZG->addr.n_sorted_elements - 1U)
      {
        zb_size_t move_len = (ZG->addr.n_sorted_elements - i - 1U);
        TRACE_MSG(TRACE_COMMON3, "move to %hd %hd %hd", (FMT__H_H_H, i, i+1, move_len));
        ZB_MEMMOVE(&ZG->addr.short_sorted[i], &ZG->addr.short_sorted[i+1U], move_len);
      }
    }
  }
  ZG->addr.n_sorted_elements -= dec;
  TRACE_MSG(TRACE_COMMON3, "dec %hd ZG->addr.n_sorted_elements %hd", (FMT__H_H, dec, ZG->addr.n_sorted_elements));
}


/**
   Add an entry into short address sorted search array

   @param ref - address ref
   @param short_addr - short address
 */
void zb_add_short_addr_sorted(zb_address_ieee_ref_t ref, zb_uint16_t short_addr)
{
  zb_ushort_t l = 0;

  if (ZG->addr.n_sorted_elements > 1U)
  {
    zb_ushort_t r = ZG->addr.n_sorted_elements;
    /* search place using binary search */
    while (l < r)
    {
      zb_ushort_t i = l + (r - l) / 2U;
      if (short_addr >= ZG->addr.addr_map[ZG->addr.short_sorted[i]].addr)
      {
        l = i + 1U;
      }
      else
      {
        r = i;
      }
    }
  }
  if (ZG->addr.n_sorted_elements > 0U
      && l < ZG->addr.n_sorted_elements
      && short_addr > ZG->addr.addr_map[ZG->addr.short_sorted[l]].addr)
  {
    l++;
  }
  if (l < ZG->addr.n_sorted_elements)
  {
    ZB_MEMMOVE(&ZG->addr.short_sorted[l+1U], &ZG->addr.short_sorted[l], ZG->addr.n_sorted_elements - l);
  }
  ZG->addr.short_sorted[l] = ref;
  ZG->addr.n_sorted_elements++;
}


/**
   Do single clock algorithm tick.

   Find first non-locked used element and clear its 'clock' field so it could be
   reused later. Update clock pointer.
 */
static void clock_tick(void)
{
  zb_address_map_t *ent;
  zb_ushort_t limit = ZG->addr.clock_i;
  zb_ushort_t cnt = 0;
  zb_bool_t found = ZB_FALSE;
  while (ZG->addr.clock_i < ZB_IEEE_ADDR_TABLE_SIZE && cnt < ZG->addr.n_elements)
  {
    ent = &ZG->addr.addr_map[ZG->addr.clock_i];
    ZG->addr.clock_i++;
    if (ZB_U2B(ent->used))
    {
      cnt++;
      if (!ZB_U2B(ent->lock_cnt))
      {
        ent->clock = 0U;
        found = ZB_TRUE;
        break;
      }
    }
  }
  if (!found && cnt < ZG->addr.n_elements)
  {
    ZG->addr.clock_i = 0;
    while (ZG->addr.clock_i < limit && cnt < ZG->addr.n_elements)
    {
      ent = &ZG->addr.addr_map[ZG->addr.clock_i];
      ZG->addr.clock_i++;
      if (ZB_U2B(ent->used))
      {
        cnt++;
        if (!ZB_U2B(ent->lock_cnt))
        {
          ent->clock = 0U;
          break;
        }
      }
    } /* while */
  } /* if */
}

zb_bool_t zb_address_compressed_cmp(zb_ieee_addr_compressed_t *one, zb_ieee_addr_compressed_t *two)
{
  return (zb_bool_t)((one)->dev_manufacturer == (two)->dev_manufacturer
          && ZB_MEMCMP(&(one)->device_id[0], &(two)->device_id[0], 5) == 0);
}


zb_uint_t zb_calc_non_zero_bits_in_bit_vector(zb_uint8_t *vector, zb_uint_t size)
{
  zb_uint_t i;
  zb_uint_t result = 0;
  for (i = 0; i < size; i++)
  {
    zb_uint8_t b = vector[i];
    b = (b & 0x55U) + ((b >> 1) & 0x55U);
    b = (b & 0x33U) + ((b >> 2) & 0x33U);
    b = (b & 0x0FU) + ((b >> 4) & 0x0FU);
    result += b;
  }
  return result;
}

void zb_address_reset(void)
{
  zb_uint8_t i;
  TRACE_MSG(TRACE_COMMON1, ">>zb_address_reset", (FMT__0));

  for (i = 0; i < ZB_IEEE_ADDR_TABLE_SIZE; i++)
  {
    zb_address_map_t *ent;

    ent = &ZG->addr.addr_map[i];
    if (!ZB_U2B(ent->used))
    {
      continue;
    }

    del_short_sorted(i);
    ent->used = ZB_FALSE_U;
    ent->lock_cnt = 0U;
    ent->clock = 0U;
  }

  ZG->addr.n_elements = 0;
  TRACE_MSG(TRACE_COMMON1, "<<zb_address_reset", (FMT__0));
}


#if defined ADDR_TABLE_DEBUG

static void print_addr_table(void)
{
  zb_ushort_t i;

  for (i=0; i<ZB_IEEE_ADDR_TABLE_SIZE; i++)
  {
    zb_address_map_t *ent;
    zb_ieee_addr_t ieee_addr;

    ent = &ZG->addr.addr_map[i];

    if (ZB_U2B(ZG->addr.addr_map[i].used))
    {
      if (ent->redirect_type == ZB_ADDR_REDIRECT_NONE)
      {
        zb_ieee_addr_decompress(ieee_addr, &ent->ieee_addr);

        TRACE_MSG(TRACE_APP1, "REGULAR: idx %hd, short 0x%x, ieee " TRACE_FORMAT_64 " back_ref %hd, lock %hd",
                  (FMT__H_D_A_H_H, i, ent->addr, TRACE_ARG_64(ieee_addr),
                   ent->redirect_ref, ent->lock_cnt));
      }
      else if (ent->redirect_type == ZB_ADDR_REDIRECT_SHORT)
      {
        TRACE_MSG(TRACE_APP1, "REDIR_SHORT: idx %hd, short 0x%x, redirect_ref %hd, lock %hd",
                  (FMT__H_D_H_H, i, ent->addr,
                   ent->redirect_ref, ent->lock_cnt));
      }
      else
      {
        zb_ieee_addr_decompress(ieee_addr, &ent->ieee_addr);

        TRACE_MSG(TRACE_APP1, "REDIR_IEEE: idx %d, ieee " TRACE_FORMAT_64 " redirect_ref %hd, lock %d",
                  (FMT__D_A_H_D, i, TRACE_ARG_64(ieee_addr), ent->redirect_ref, ent->lock_cnt));
      }
    }
  }
}
#endif  /* # if defined ADDR_TABLE_DEBUG */

zb_bool_t zb_address_cmp_two_refs(zb_address_ieee_ref_t addr_ref_a, zb_address_ieee_ref_t addr_ref_b)
{
  return (addr_ref_a == addr_ref_b)
          /* Ref A is a redirect ref to B. */
          || (ZB_U2B(ZG->addr.addr_map[addr_ref_a].used)
              && ZG->addr.addr_map[addr_ref_a].redirect_type != ZB_ADDR_REDIRECT_NONE
              && ZG->addr.addr_map[addr_ref_a].redirect_ref == addr_ref_b)
          /* Ref B is a redirect ref to A. */
          || (ZB_U2B(ZG->addr.addr_map[addr_ref_a].used)
              && ZG->addr.addr_map[addr_ref_b].redirect_type != ZB_ADDR_REDIRECT_NONE
              && ZG->addr.addr_map[addr_ref_b].redirect_ref == addr_ref_a);
}

zb_bool_t zb_address_in_use(zb_address_ieee_ref_t ref)
{
  return ZB_U2B(ZG->addr.addr_map[ref].used);
}

#endif /* ZB_MACSPLIT_DEVICE */

/*! @} */
