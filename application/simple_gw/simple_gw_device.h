/*
 * ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2022 DSR Corporation, Denver CO, USA.
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
/* PURPOSE: Simple GW device definition
*/
#ifndef SIMPLE_GW_DEVICE_H
#define SIMPLE_GW_DEVICE_H 1

#define ZB_DEVICE_VER_SIMPLE_GW 0  /*!< Simple GW device version */
#define ZB_SIMPLE_GW_DEVICE_ID 0

#define ZB_SIMPLE_GW_IN_CLUSTER_NUM 3  /*!< @internal Simple GW IN clusters number */
#define ZB_SIMPLE_GW_OUT_CLUSTER_NUM 5 /*!< @internal Simple GW OUT clusters number */

#define ZB_SIMPLE_GW_CLUSTER_NUM                                      \
  (ZB_SIMPLE_GW_IN_CLUSTER_NUM + ZB_SIMPLE_GW_OUT_CLUSTER_NUM)

/*! @internal Number of attribute for reporting on Simple GW device */
#define ZB_SIMPLE_GW_REPORT_ATTR_COUNT \
  (ZB_ZCL_ON_OFF_SWITCH_CONFIG_REPORT_ATTR_COUNT)

/**
 *  @brief Declare attribute list for Identify cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_IDENTIFY_CLIENT_ATTRIB_LIST(attr_list)			      \
	ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_IDENTIFY) \
	ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for Scenes cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_SCENES_CLIENT_ATTRIB_LIST(attr_list)			    \
	ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_SCENES) \
	ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for Groups cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_GROUPS_CLIENT_ATTRIB_LIST(attr_list)			    \
	ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_GROUPS) \
	ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for On/Off cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_ON_OFF_CLIENT_ATTRIB_LIST(attr_list)			    \
	ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_ON_OFF) \
	ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 *  @brief Declare attribute list for IAS Zone cluster (client).
 *  @param attr_list - attribute list name.
 */
#define ZB_ZCL_DECLARE_IAS_ZONE_CLIENT_ATTRIB_LIST(attr_list)			    \
	ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_IAS_ZONE) \
	ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**
 * @brief Declare cluster list for Simple GW device
 * @param cluster_list_name - cluster list variable name
 * @param basic_attr_list - attribute list for Basic cluster (server role)
 * @param identify_client_attr_list - attribute list for Identify cluster (client role)
 * @param identify_attr_list - attribute list for Identify cluster (server role)
 * @param scenes_client_attr_list - attribute list for Scenes cluster (client role)
 * @param groups_client_attr_list - attribute list for Groups cluster (client role)
 * @param on_off_client_attr_list - attribute list for On/Off cluster (client role)
 * @param on_off_switch_config_attr_list - attribute list for On/off switch configuration cluster (server role)
 * @param ias_zone_client_attr_list - attribute list for IAS Zone cluster (client role)
 */
#define ZB_DECLARE_SIMPLE_GW_CLUSTER_LIST(                                  \
      cluster_list_name,                                                    \
      basic_attr_list,                                                      \
      identify_client_attr_list,                                            \
      identify_attr_list,                                                   \
      scenes_client_attr_list,                                              \
      groups_client_attr_list,                                              \
      on_off_client_attr_list,                                              \
      on_off_switch_config_attr_list,                                       \
      ias_zone_client_attr_list)                                            \
      zb_zcl_cluster_desc_t cluster_list_name[] =                           \
      {                                                                     \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG,                           \
          ZB_ZCL_ARRAY_SIZE(on_off_switch_config_attr_list, zb_zcl_attr_t), \
          (on_off_switch_config_attr_list),                                 \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_IDENTIFY,                                       \
          ZB_ZCL_ARRAY_SIZE(identify_attr_list, zb_zcl_attr_t),             \
          (identify_attr_list),                                             \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_BASIC,                                          \
          ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),                \
          (basic_attr_list),                                                \
          ZB_ZCL_CLUSTER_SERVER_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_ON_OFF,                                         \
          ZB_ZCL_ARRAY_SIZE(on_off_client_attr_list, zb_zcl_attr_t),        \
          (on_off_client_attr_list),                                        \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_SCENES,                                         \
          ZB_ZCL_ARRAY_SIZE(scenes_client_attr_list, zb_zcl_attr_t),        \
          (scenes_client_attr_list),                                        \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_GROUPS,                                         \
          ZB_ZCL_ARRAY_SIZE(groups_client_attr_list, zb_zcl_attr_t),        \
          (groups_client_attr_list),                                        \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        ),                                                                  \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_IAS_ZONE,                                       \
          ZB_ZCL_ARRAY_SIZE(ias_zone_client_attr_list, zb_zcl_attr_t),      \
          (ias_zone_client_attr_list),                                      \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
          ),                                                                \
        ZB_ZCL_CLUSTER_DESC(                                                \
          ZB_ZCL_CLUSTER_ID_IDENTIFY,                                       \
          ZB_ZCL_ARRAY_SIZE(identify_client_attr_list, zb_zcl_attr_t),      \
          (identify_client_attr_list),                                      \
          ZB_ZCL_CLUSTER_CLIENT_ROLE,                                       \
          ZB_ZCL_MANUF_CODE_INVALID                                         \
        )                                                                   \
    }


/** @internal @brief Declare simple descriptor for Simple GW device
    @param ep_name - endpoint variable name
    @param ep_id - endpoint ID
    @param in_clust_num - number of supported input clusters
    @param out_clust_num - number of supported output clusters
    @note in_clust_num, out_clust_num should be defined by numeric constants, not variables or any
    definitions, because these values are used to form simple descriptor type name
*/
#define ZB_ZCL_DECLARE_SIMPLE_GW_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num) \
  ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);                                    \
  ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =             \
  {                                                                                       \
    ep_id,                                                                                \
    ZB_AF_HA_PROFILE_ID,                                                                  \
    ZB_SIMPLE_GW_DEVICE_ID,                                                               \
    ZB_DEVICE_VER_SIMPLE_GW,                                                              \
    0,                                                                                    \
    in_clust_num,                                                                         \
    out_clust_num,                                                                        \
    {                                                                                     \
      ZB_ZCL_CLUSTER_ID_BASIC,                                                            \
      ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                         \
      ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG,                                             \
      ZB_ZCL_CLUSTER_ID_ON_OFF,                                                           \
      ZB_ZCL_CLUSTER_ID_SCENES,                                                           \
      ZB_ZCL_CLUSTER_ID_GROUPS,                                                           \
      ZB_ZCL_CLUSTER_ID_IAS_ZONE,                                                         \
      ZB_ZCL_CLUSTER_ID_IDENTIFY,                                                         \
    }                                                                                     \
  }

/** @brief Declare endpoint for Simple GW device
    @param ep_name - endpoint variable name
    @param ep_id - endpoint ID
    @param cluster_list - endpoint cluster list
 */
#define ZB_DECLARE_SIMPLE_GW_EP(ep_name, ep_id, cluster_list) \
  ZB_ZCL_DECLARE_SIMPLE_GW_SIMPLE_DESC(                       \
      ep_name,                                                \
      ep_id,                                                  \
      ZB_SIMPLE_GW_IN_CLUSTER_NUM,                            \
      ZB_SIMPLE_GW_OUT_CLUSTER_NUM);                          \
  ZB_AF_DECLARE_ENDPOINT_DESC(ep_name,                        \
      ep_id,                                                  \
      ZB_AF_HA_PROFILE_ID,                                    \
      0,                                                      \
      NULL,                                                   \
      ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t), \
      cluster_list,                                           \
      (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name,       \
      0, NULL, /* No reporting ctx */                         \
      0, NULL)

/** @brief Declare Simple GW device context.
    @param device_ctx - device context variable name.
    @param ep_name - endpoint variable name.
*/
#define ZB_DECLARE_SIMPLE_GW_CTX(device_ctx, ep_name)  ZBOSS_DECLARE_DEVICE_CTX_1_EP(device_ctx, ep_name)
 /* No CVC ctx */

/*! @} */

#endif /* SIMPLE_GW_DEVICE_H */
