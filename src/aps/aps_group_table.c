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
/* PURPOSE: Functions for Group Table Management.
*/

#define ZB_TRACE_FILE_ID 101

#include "zb_common.h"
#include "zb_aps.h"

zb_ret_t zb_aps_group_table_add(zb_aps_group_table_t *table, zb_uint16_t group, zb_uint8_t ep)
{
  zb_ushort_t i, j;
  zb_ret_t ret = (zb_ret_t)ZB_APS_STATUS_SUCCESS;
  for (i = 0 ; i < table->n_groups; ++i)
  {
    if (table->groups[i].group_addr == group)
    {
      for (j = 0 ; j < table->groups[i].n_endpoints ; ++j)
      {
        if (table->groups[i].endpoints[j] == ep)
        {
          goto done;
        }
      }
      if (j < ZB_APS_ENDPOINTS_IN_GROUP_TABLE)
      {
        TRACE_MSG(TRACE_APS3, "add endpoint: ind = %d", (FMT__H, j));
        table->groups[i].endpoints[j] = ep;
        table->groups[i].n_endpoints++;
      }
      else
      {
        TRACE_MSG(TRACE_ERROR, "no more space for endpoints", (FMT__0));
        ret = ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_TABLE_FULL);
      }
      goto done;
    }
  }
  if (i < ZB_APS_GROUP_TABLE_SIZE)
  {
    table->groups[i].group_addr = group;
    table->groups[i].endpoints[0] = ep;
    table->groups[i].n_endpoints = 1;
    TRACE_MSG(TRACE_APS3, "add group: ind = %d, group: 0x%04x", (FMT__H_D, i, group));
    table->n_groups++;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "no more space for groups", (FMT__0));
    ret = ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_TABLE_FULL);
  }
  done:
  TRACE_MSG(TRACE_ERROR, "add group ret: %d", (FMT__H, ret));
  return ret;
}


zb_ret_t zb_aps_group_table_remove(zb_aps_group_table_t *table, zb_uint16_t group, zb_uint8_t ep)
{
  zb_uindex_t i, j;

  for (i = 0; i < table->n_groups; ++i)
  {
    if (table->groups[i].group_addr == group)
    {
      for (j = 0; j < table->groups[i].n_endpoints; ++j)
      {
        if (table->groups[i].endpoints[j] == ep)
        {
          if (table->groups[i].n_endpoints > 1U)
          {
            /* Move others ep entries to the left */
            ZB_MEMMOVE( &(table->groups[i].endpoints[j]),
                        &(table->groups[i].endpoints[j+1U]),
                        (table->groups[i].n_endpoints - j - 1U)*sizeof(table->groups[i].endpoints[0]) );
            --table->groups[i].n_endpoints;
          }
          else
          {
            /* Remove group entry */
            if (i < ((zb_uindex_t)table->n_groups - 1U))
            {
              ZB_MEMMOVE( &(table->groups[i]),
                          &(table->groups[i+1U]),
                          (table->n_groups - i - 1U)*sizeof(zb_aps_group_table_ent_t));
            }
            --table->n_groups;
          }

          TRACE_MSG(TRACE_APS2, "remove group %d for endpoint %hd", (FMT__D_H, group, ep));
          return (zb_ret_t)ZB_APS_STATUS_SUCCESS;
        }
      }
    }
  }
  TRACE_MSG(TRACE_ERROR, "not found group %d for endpoint %hd", (FMT__D_H, group, ep));
  return ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_INVALID_GROUP);
}


void zb_aps_group_table_remove_all(zb_aps_group_table_t *table)
{
  ZB_BZERO(table, sizeof(*table));
}


zb_bool_t zb_aps_group_table_is_endpoint_in_group(zb_aps_group_table_t *table,
                                                  zb_uint16_t group_id,
                                                  zb_uint8_t endpoint)
{
  zb_bool_t result = (group_id==0U); // (group_id 0 - specific group) // ZB_FALSE;
  zb_uint8_t i,j;

  TRACE_MSG(
      TRACE_APS1,
      "> zb_aps_group_table_is_endpoint_in_group group_id 0x%04x endpoint %hd",
      (FMT__D_H, group_id, endpoint));

  for (i = 0; (i < table->n_groups) && !result; ++i)
  {
    if (table->groups[i].group_addr == group_id)
    {
      for (j = 0; j < table->groups[i].n_endpoints; ++j)
      {
        if (table->groups[i].endpoints[j] == (zb_uint8_t)endpoint)
        {
          result = ZB_TRUE;
          /* Breaking two loops at once */
          goto zb_aps_is_endpoint_in_group_finish;
        }
      }
    }
  }

zb_aps_is_endpoint_in_group_finish:
  TRACE_MSG(
      TRACE_APS1,
      "< zb_aps_group_table_is_endpoint_in_group result %hd",
      (FMT__H, result));

  return result;
}


static void zb_aps_group_table_get_membership(zb_aps_group_table_t *table,
                                       zb_ushort_t in_group_cnt, zb_uint16_t *in_groups,
                                       zb_ushort_t *out_group_cnt, zb_uint16_t *out_groups,
                                       zb_ushort_t *out_capacity)
{
  zb_uint8_t i, j;

  *out_capacity = ZB_APS_GROUP_TABLE_SIZE - (zb_ushort_t)table->n_groups;
  *out_group_cnt = 0;
  for (i = 0; i < in_group_cnt; ++i)
  {
    for (j = 0; j < table->n_groups; ++j)
    {
      if (table->groups[j].group_addr == in_groups[i])
      {
        out_groups[*out_group_cnt] = table->groups[j].group_addr;
        *out_group_cnt += 1U;
        break;
      }
    }
  }

  if (in_group_cnt == 0U)
  {
    for (j = 0; j < table->n_groups; ++j)
    {
      out_groups[*out_group_cnt] = table->groups[j].group_addr;
      *out_group_cnt += 1U;
    }
  }
}

void zb_apsme_internal_get_group_membership_request(zb_aps_group_table_t *table, zb_uint8_t param)
{
  zb_apsme_get_group_membership_req_t *req;
  zb_apsme_get_group_membership_conf_t conf;

  req = (zb_apsme_get_group_membership_req_t*)zb_buf_begin(param);

  TRACE_MSG(TRACE_APS3, ">>zb_apsme_get_group_membership_internal", (FMT__0));

  /* Check that req->groups is properly aligned. */
  ZB_ASSERT((((zb_uint64_t)&req->groups[0]) % sizeof(req->groups[0])) == 0U);
  zb_aps_group_table_get_membership(table,
                                    req->n_groups, (zb_uint16_t *)(void *)req->groups,/*Casting to zb_uint16_t * via interim void * to prevent MISRA violation*/
                                    &conf.n_groups, conf.groups,
                                    &conf.capacity);

  ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_apsme_get_group_membership_conf_t),
            &conf, sizeof(zb_apsme_get_group_membership_conf_t));
  TRACE_MSG(TRACE_APS3, "<<zb_apsme_get_group_membership_internal: capacity=%hd; n_groups=%hd;", (FMT__H_H, conf.capacity, conf.n_groups));

  /* Run user confirm function - ZCL-GET-GROUP-MEMBERSHIP.confirm via callback */
  if (req->confirm_cb != NULL)
  {
    ZB_SCHEDULE_CALLBACK(req->confirm_cb, param);
  }
  else
  {
    zb_buf_free(param);
  }

  TRACE_MSG(TRACE_APS3, "<<zb_apsme_get_group_membership_internal", (FMT__0));
}
