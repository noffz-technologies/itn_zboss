#include "zboss_api.h"

// start
typedef struct start_params_s
{
  zb_uint8_t port;
  zb_uint8_t role;
  zb_uint64_t ieee_addr;
  zb_uint8_t nwk_key[16];
  zb_uint32_t channel_mask;
  zb_uint16_t pan_id;
  zb_bool_t erase;
  zb_bool_t ic_only;
  zb_uint8_t local_ic_type; // 4 for no IC
  zb_uint8_t *local_ic; 
  zb_bool_t rx_on_when_idle;
  zb_uint8_t ed_timeout;
  zb_uint32_t poll_interval;
  } start_params_t;

typedef struct start_resp_s
{
  zb_ret_t status;
  zb_uint16_t addr;
  zb_uint8_t nwk_key[48]; // 3*16 bytes
  zb_uint8_t channel;
  zb_uint16_t pan_id;
  zb_uint64_t ext_pan_id;
} start_resp_t;

typedef struct install_code_s
{
  zb_uint64_t ieee_addr;
  zb_uint8_t ic_type;
  zb_uint8_t *ic; 
} install_code_t;

typedef ZB_PACKED_PRE struct zb_aps_installcode_nvram_s
{
  zb_ieee_addr_t  device_address;               /*!< Partner address */
  /*AEV:Here data was a filler: align[2]; - remove it for storage of IC type for now.
    options lowest 2 bits [0-1]: 00-48, 01-64, 10-96, 11-128 bits ic type.*/
  zb_uint8_t      options;
  zb_uint8_t      align;
  zb_uint8_t      installcode[ZB_CCM_KEY_SIZE+ZB_CCM_KEY_CRC_SIZE];       /*!< 16b installcode +2b crc */
  /*hint:we can remove 2b crc at the end of installcode, but we must keep ic type, thus align to dword and don't change it*/
} ZB_PACKED_STRUCT zb_aps_installcode_nvram_t;

typedef struct secur_ic_get_list_resp_s
{
  zb_uint8_t status;
  zb_uint8_t entries;
  zb_uint8_t count;
  zb_aps_installcode_nvram_t *list;
} secur_ic_get_list_resp_t;

typedef struct mgmt_leave_req_s
{
  zb_uint16_t address;
  zb_bool_t rejoin;
  zb_bool_t remove_children;
} mgmt_leave_req_t;

typedef ZB_PACKED_PRE struct mgmt_lqi_resp_s
{
  zb_uint8_t status;                     /*!< The status of the Mgmt_Lqi_req command.*/
  zb_uint8_t neighbor_table_entries;                     /*!< Number of Neighbor Table entries.*/
  zb_uint8_t neighbor_table_list_count;  /*!< Number of Neighbor Table
                                          * entries included within NeighborTableList*/
  zb_zdo_neighbor_table_record_t *list;
}
ZB_PACKED_STRUCT
mgmt_lqi_resp_t;

typedef struct ep_resp_s{
  zb_zdo_ep_resp_t response;
  zb_uint8_t *array;
} ZB_PACKED_STRUCT ep_resp_t;

typedef struct simple_desc_s 
{                        
  zb_uint8_t    status;                   /*!< The status of the Desc_req command. @ref zdp_status */
  zb_uint16_t   nwk_addr;                 /*!< NWK address for the request  */
  zb_uint8_t    length;                   /*!< Length of the simple descriptor */                                                                 
  zb_uint8_t    endpoint;                 /* Endpoint */                                
  zb_uint16_t   app_profile_id;           /* Application profile identifier */          
  zb_uint16_t   app_device_id;            /* Application device identifier */           
  zb_uint8_t    app_device_version;       /* Application device version */                 
  zb_uint8_t    app_input_cluster_count;  /* Application input cluster count */         
  zb_uint8_t    app_output_cluster_count; /* Application output cluster count */        
  zb_uint16_t   *app_cluster_list;        /* Application input and output cluster list */ 
} ZB_PACKED_STRUCT simple_desc_t;

typedef struct zcl_cmd_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint8_t    command; 
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zb_uint8_t    direction;
  zb_uint8_t    size;
  zb_uint8_t    *data;
} zcl_cmd_req_t;

typedef struct zcl_read_attr_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint16_t   attribute;
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
} zcl_read_attr_req_t;

typedef struct zcl_read_attr_resp_s
{
  zb_uint16_t attr_id;      
  zb_uint8_t  status;        
  zb_uint8_t  attr_type;     
  zb_uint8_t  attr_length;     
  zb_uint8_t  *array;
} ZB_PACKED_STRUCT zcl_read_attr_resp_t;

typedef struct zcl_write_attr_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint16_t   attribute;  // attribute, one should be populated, other 0
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zb_uint8_t    attr_type;
  zb_uint8_t    *data;
} zcl_write_attr_req_t;

typedef struct zcl_disc_attr_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep;   
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zb_uint16_t   start_attr_id;
  zb_uint8_t    max;
} zcl_disc_attr_req_t;

typedef struct zcl_disc_attr_resp_s
{
  zb_uint8_t                complete;
  zb_uint8_t                length;
  zb_zcl_disc_attr_info_t   *list;
} zcl_disc_attr_resp_t;

typedef struct zcl_cmd_info_s
{
  zb_uint16_t               source;
  zb_uint8_t                ep;
  zb_uint16_t               cluster;
  zb_uint8_t                cmd;
  zb_bool_t                 disable_default_response;
  zb_bool_t                 is_manuf_specific;
  zb_uint8_t                payload_length;
  zb_uint8_t                *payload;
} zcl_cmd_info_t;

typedef struct zcl_reporting_configuration_s
{
  zb_uint16_t   attr_id;
  zb_uint8_t    attr_type;
  zb_uint16_t   min_interval;
  zb_uint16_t   max_interval;
  zb_uint64_t   change;
} zcl_reporting_configuration_t;

typedef struct zcl_configure_reporting_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zcl_reporting_configuration_t configuration;
} zcl_configure_reporting_req_t;

typedef struct zcl_read_reporting_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zb_uint16_t   attribute;
} zcl_read_reporting_req_t;

typedef struct zcl_read_reporting_res_s
{
  zb_uint8_t status; // FF if direction is not correct
  zcl_reporting_configuration_t config;
} zcl_read_reporting_res_t;

typedef struct zcl_discover_commands_req_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    dst_addr_mode;
  zb_uint8_t    dst_ep;
  zb_uint8_t    src_ep; 
  zb_uint16_t   profile_id;
  zb_uint16_t   cluster_id;
  zb_uint8_t    disable_default_response;
  zb_uint8_t    is_manuf_specific;
  zb_uint16_t   manuf_code;
  zb_uint8_t    start_cmd_id;
  zb_uint8_t    max_len;
} zcl_discover_commands_req_t;

typedef struct zcl_discover_commands_resp_s
{
  zb_uint8_t length;
  zb_uint8_t *list;
} zcl_discover_commands_resp_t;

typedef struct attribute_report_s
{
  zb_uint64_t   nwk_addr; 
  zb_uint8_t    ep; 
  zb_uint16_t   cluster_id;
  zb_uint16_t   attribute; 
  zb_uint8_t    attr_type;
  zb_uint8_t    data_size;
  zb_uint8_t    *data;
} attribute_report_t;

typedef struct binding_table_resp_s
{
  zb_uint8_t status;
  zb_uint8_t entries;
  zb_uint8_t count;
  zb_zdo_binding_table_record_t *list;
} binding_table_resp_t;

typedef struct address_index_s
{
  zb_uint16_t address;
  zb_uint8_t index;
} address_index_t;

// callbacks
typedef void (*log_cbt)(const char* str);
typedef void (*noparam_cbt)();
typedef void (*ret_cbt)(zb_ret_t param);
typedef void (*start_cbt)(start_resp_t param);
typedef void (*secur_ic_list_cbt)(secur_ic_get_list_resp_t param);
typedef void (*mgmt_lqi_req_cbt)(mgmt_lqi_resp_t param);
typedef void (*nwk_addr_req_cbt)(zb_zdo_nwk_addr_resp_head_t param);
typedef void (*get_diag_data_cbt)(zb_zdo_get_diag_data_resp_params_t param);
typedef void (*active_ep_req_cbt)(ep_resp_t param);
typedef void (*simple_desc_req_cbt)(simple_desc_t param);
typedef void (*read_attr_cbt)(zcl_read_attr_resp_t param);
typedef void (*disc_attr_cbt)(zcl_disc_attr_resp_t param);
typedef void (*binding_table_cbt)(binding_table_resp_t param);
typedef void (*read_reporting_cbt)(zcl_read_reporting_res_t param);
typedef void (*discover_commands_cbt)(zcl_discover_commands_resp_t param);
typedef void (*get_tx_power_cbt)(zb_int8_t param);
typedef void (*device_annce_cbt)(zb_zdo_signal_device_annce_params_t param);
typedef void (*device_leave_indication_cbt)(zb_zdo_signal_leave_indication_params_t param);
typedef void (*cmd_recive_cbt)(zcl_cmd_info_t param);
typedef void (*attribute_report_cbt)(attribute_report_t param);

typedef zb_bool_t (*zb_error_handler_t)(zb_uint8_t severity,
                                        zb_ret_t error_code,
                                        void *additional_info);

typedef struct callbacks_s{
  zb_error_handler_t error;
  log_cbt log;
  start_cbt zboss_start;
  noparam_cbt leave_signal;
  device_annce_cbt device_annce;
  device_leave_indication_cbt device_leave_indication;
  cmd_recive_cbt cmd_receive;
  attribute_report_cbt attribute_report;
  ret_cbt mgmt_permit_joining_req;
  ret_cbt secur_ic_add;
  secur_ic_list_cbt secur_ic_list;
  ret_cbt secur_ic_remove_all;
  ret_cbt mgmt_leave_req;
  mgmt_lqi_req_cbt mgmt_lqi_req;
  nwk_addr_req_cbt nwk_addr_req;
  get_diag_data_cbt get_diag_data;
  active_ep_req_cbt active_ep_req;
  simple_desc_req_cbt simple_desc_req;
  ret_cbt zcl_send_cmd;
  read_attr_cbt attr_read;
  ret_cbt attr_write;
  disc_attr_cbt attr_disc;
  ret_cbt bind_req;
  ret_cbt unbind_req;
  binding_table_cbt binding_table;
  ret_cbt configure_reporting;
  read_reporting_cbt read_reporting;
  discover_commands_cbt discover_commands;
  ret_cbt set_tx_power;
  get_tx_power_cbt get_tx_power;
} ZB_PACKED_STRUCT callbacks_t;


