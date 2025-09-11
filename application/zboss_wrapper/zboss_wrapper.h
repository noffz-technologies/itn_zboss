#include "zboss_api.h"
#include "types.h"

#define ZB_TRACE_FILE_ID 66666
#define DEVICE_IEEE_ADDR {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa}
#define DEVICE_CHANNEL_MASK (1l<<16)
#define DEVICE_NWK_KEY { 0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89, 0, 0, 0, 0, 0, 0, 0, 0} //AB:CD:EF:01:23:45:67:89:0:0:0:0:0:0:0:0 ABCDEF012345678900000000
#define ENDPOINT_ID 1


void zdo_send_parent_annce_at_formation() {
        printf("lol");
};

uint8_t device_handler(uint8_t param);
void device_cb(uint8_t param);

/* Basic cluster attributes */
zb_uint8_t attr_zcl_version  = ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE;
zb_uint8_t attr_power_source = ZB_ZCL_BASIC_POWER_SOURCE_DEFAULT_VALUE;
zb_uint16_t identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;

/* Declare attribute list for Basic cluster (server). */
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST(basic_attr_list, &attr_zcl_version, &attr_power_source);
ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(ident_attr_list, &identify_time);

// Define the list of clusters manually (server and client roles)
zb_zcl_cluster_desc_t clusters[] = {
    {
        ZB_ZCL_CLUSTER_ID_BASIC, // Cluster ID for Basic cluster
        sizeof(basic_attr_list) / sizeof(basic_attr_list[0]),  // Number of attributes in the basic_attr_list
        basic_attr_list,  // Pointer to the attribute list
        ZB_ZCL_CLUSTER_SERVER_ROLE,  // This cluster is in the server role
        ZB_ZCL_MANUF_CODE_INVALID    // No manufacturer-specific code
    },
    {
        ZB_ZCL_CLUSTER_ID_IDENTIFY,  // Cluster ID for Identify cluster
        1,
        ident_attr_list,
        ZB_ZCL_CLUSTER_SERVER_ROLE,  // Server role
        ZB_ZCL_MANUF_CODE_INVALID    // No manufacturer-specific code
    },
    {
        ZB_ZCL_CLUSTER_ID_BASIC, // Cluster ID for Basic cluster
        0,  // Number of attributes in the basic_attr_list
        NULL,  // Pointer to the attribute list
        ZB_ZCL_CLUSTER_CLIENT_ROLE,  // This cluster is in the server role
        ZB_ZCL_MANUF_CODE_INVALID    // No manufacturer-specific code
    },
    {
        ZB_ZCL_CLUSTER_ID_ON_OFF, // Cluster ID for On/Off cluster
        0,  // Number of attributes in the basic_attr_list
        NULL,  // Pointer to the attribute list
        ZB_ZCL_CLUSTER_CLIENT_ROLE,  // This cluster is in the server role
        ZB_ZCL_MANUF_CODE_INVALID    // No manufacturer-specific code
    }
    // Add more clusters here for client and server roles...
};

// Define a simple descriptor manually
zb_af_simple_desc_1_1_t simple_desc = {
    .endpoint = 1,  // Endpoint ID
    .app_profile_id = ZB_AF_HA_PROFILE_ID,  // Home Automation Profile ID
    .app_device_id = 0,  // Device ID
    .app_device_version = 0,  // Device version
    .reserved = 0,
    .app_input_cluster_count = 2,  // Number of input clusters
    .app_output_cluster_count = 2, // Number of output clusters
    .app_cluster_list = {
        ZB_ZCL_CLUSTER_ID_BASIC,    // Input cluster IDs
        ZB_ZCL_CLUSTER_ID_IDENTIFY,
        ZB_ZCL_CLUSTER_ID_BASIC,    // Output cluster IDs
        ZB_ZCL_CLUSTER_ID_ON_OFF,   
    }
};

zb_af_endpoint_desc_t endpoint = {
    .ep_id = ENDPOINT_ID,  // Endpoint ID
    .profile_id = ZB_AF_HA_PROFILE_ID,  // Profile ID
    .device_handler = device_handler,
    .identify_handler = device_cb,
    .reserved_size = 0,
    .reserved_ptr = 0,
    .cluster_count = 4,
    .cluster_desc_list = clusters,  // Cluster descriptions
    .simple_desc = &simple_desc,  // Pointer to the simple descriptor
};

zb_af_endpoint_desc_t *endpoints[1] = {&endpoint};

// Define the device context manually (with one endpoint)
zb_af_device_ctx_t device_ctx = {
    .ep_count = 1,
    .ep_desc_list = endpoints
};