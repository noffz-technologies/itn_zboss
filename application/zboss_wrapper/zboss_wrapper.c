#include "zboss_wrapper.h"
#include "signal.h"

// GLOBAL VARIABLES
char portname[128] = "/dev/ttyACM0";
zb_uint8_t device_role;
zb_time_t poll_interval;
zb_ieee_addr_t g_ieee_addr = DEVICE_IEEE_ADDR;
zb_uint8_t g_key_nwk[16] = DEVICE_NWK_KEY;
zb_uint8_t *local_ic = NULL;
start_resp_t start_response;

const zb_uint8_t zb_ic_size_by_type[ZB_IC_TYPE_MAX]={6,8,12,16};

callbacks_t callbacks = {};
bool custom_device_context_available = false;
zb_af_device_ctx_t custom_device_context;

// parameter passing
install_code_t secur_ic_add_req_par;
zb_uint8_t *add_ic = NULL;
address_index_t address_index_lqi;
address_index_t address_index_bindingtable;
mgmt_leave_req_t mgmt_leave_req_par;
zb_zdo_nwk_addr_req_param_t nwk_addr_req_par;
zb_zdo_simple_desc_req_t simple_desc_req_par;
zcl_cmd_req_t zcl_cmd_req_par;
zcl_read_attr_req_t zcl_read_attr_req_par;
zcl_write_attr_req_t zcl_write_attr_req_par;
zcl_disc_attr_req_t zcl_disc_attr_req_par;
zb_zdo_bind_req_param_t bind_req_par;
zcl_configure_reporting_req_t configure_reporting_par;
zcl_read_reporting_req_t read_reporting_par;
zcl_discover_commands_req_t discover_commands_par;


// hardcoded ics - not used
zb_uint8_t ithinx_euid[8] = { 0x06, 0xC5, 0x7C, 0x02, 0x09, 0x5E, 0x1E, 0x00 };
zb_uint8_t ithinx_ic[18] = {
    0x11, 0x77, 0x3A, 0x95, 0x14, 0x84, 0x1E, 0xE0,
    0x2B, 0x3E, 0x83, 0x71, 0xE8, 0x72, 0xEA, 0x48,
    0x01, 0x58
};

void handle_signal(int signal) {
    if (signal == SIGSEGV) {
        fprintf(stderr, "Segmentation fault occurred!\n");
        exit(EXIT_FAILURE); // Exit cleanly instead of crashing
    }
}

void setup_signal_handler() {
    signal(SIGSEGV, handle_signal);
}

const char* zb_mac_uart_path(void)
{
    TRACE_MSG(TRACE_APP1, portname, (FMT__0));
    return portname;
}

void device_cb(zb_bufid_t buf)
{
    //zb_zcl_device_callback_param_t *params = ZB_BUF_GET_PARAM(buf, zb_zcl_device_callback_param_t);
    TRACE_MSG(TRACE_APP1, "Device CB received, param: %hd.", (FMT__H,buf));
}
void identify_cb(zb_uint8_t buf)
{
    TRACE_MSG(TRACE_APP1, "identify_cb called, param: %hd.", (FMT__H,buf));
}
void report_attribute_cb(zb_zcl_addr_t *addr, zb_uint8_t ep, zb_uint16_t cluster_id,
                              zb_uint16_t attr_id, zb_uint8_t attr_type, zb_uint8_t *value)
{
    TRACE_MSG(TRACE_APP1, "Attribute report received: addr: %x, ep: %hd, cluster_id: %d, attr_id: %d, attr_type: %hd",(FMT__D_H_D_D_H,addr->u.short_addr,ep,cluster_id,attr_id,attr_type));
    
    attribute_report_t report = {.nwk_addr = addr->u.short_addr, .ep = ep, .cluster_id = cluster_id, .attribute = attr_id, .attr_type = attr_type, .data_size = zb_zcl_get_attribute_size(attr_type,value), .data = value};
    callbacks.attribute_report(report);
}

int register_device_context(zb_af_device_ctx_t ctx)
{
    TRACE_MSG(TRACE_APP1, "register_device_ctx called", (FMT__0));
    custom_device_context = ctx;
    for(int i=0;i<ctx.ep_count;i++)
    {
        custom_device_context.ep_desc_list[i]->device_handler = device_handler;
        custom_device_context.ep_desc_list[i]->identify_handler = identify_cb;
    }
    zb_af_register_device_ctx(&custom_device_context);
    custom_device_context_available = true;
    return 0;
}
void init(callbacks_t list,start_params_t params)
{
    ZB_SET_TRACE_ON();
    ZB_INIT("/var/log/zboss_wrapper");
    zb_set_nvram_erase_at_start(params.erase);

    TRACE_MSG(TRACE_APP1, "Init started...", (FMT__0));
    strncpy(portname, (const char*)params.port, sizeof(portname)-1);
    callbacks = list;
    
    zb_error_register_app_handler(callbacks.error);
    device_role = params.role;
    poll_interval = params.poll_interval;
    memcpy(g_ieee_addr, &params.ieee_addr, 8);
    memcpy(g_key_nwk, params.nwk_key, 16);

    zb_set_long_address(g_ieee_addr);
    if(params.pan_id != 0xFFFF)
        zb_set_pan_id(params.pan_id);
    memcpy(g_key_nwk, params.nwk_key, 16);
    
    switch (params.role)
    {
        case ZB_NWK_DEVICE_TYPE_COORDINATOR: 
            zb_set_network_coordinator_role(params.channel_mask);
            zb_secur_setup_nwk_key((zb_uint8_t *) g_key_nwk, 0);
            zb_set_installcode_policy(params.ic_only);
            break;
        case ZB_NWK_DEVICE_TYPE_ROUTER:
            zb_set_network_router_role(params.channel_mask);
            break;
        case ZB_NWK_DEVICE_TYPE_ED:
            zb_set_network_ed_role(params.channel_mask);
            // ZED-specific settings
            zb_set_ed_timeout(params.ed_timeout);
            zb_set_rx_on_when_idle(params.rx_on_when_idle);
            break;
    }
    if(params.role != ZB_NWK_DEVICE_TYPE_COORDINATOR && params.local_ic_type < 4)
    {
        zb_uint8_t size = zb_ic_size_by_type[params.local_ic_type];
        local_ic = malloc(size + 2);
        memcpy(local_ic,params.local_ic,size);
        zb_uint16_t crc = ~zb_crc16(params.local_ic, 0xffff, size);
        memcpy(local_ic + size, &crc, 2);
        zb_secur_ic_set(params.local_ic_type,local_ic);
    }
    
}
void start()
{ 
    /* Trace enable */

    if(!custom_device_context_available)
    {
        TRACE_MSG(TRACE_ERROR, "Custom device context unavailable", (FMT__0));
        ZB_AF_REGISTER_DEVICE_CTX(&device_ctx);
    }
    ZB_ZCL_REGISTER_DEVICE_CB(device_cb);
    ZB_ZCL_SET_REPORT_ATTR_CB(report_attribute_cb);
    
    zb_zcl_identify_init_server();
    zb_zcl_identify_init_client();

    if (zboss_start() != RET_OK)
    {
        TRACE_MSG(TRACE_ERROR, "ERROR dev_start failed", (FMT__0));
    }
    else
    {
        /* Call the main loop */
        zboss_main_loop();
    }
    TRACE_DEINIT();
}
void setpolicy(){
    //To change TC Link Keys Required policy, change value at zdo_commissioning_bdb.c:239, in bdb_init().
    //Right now it is set to false on ZC, and true on ZED and ZR.
    zb_secur_set_tc_rejoin_enabled(ZB_TRUE);
    zb_secur_set_unsecure_tc_rejoin_enabled(ZB_TRUE);
    
    if(device_role == ZB_NWK_DEVICE_TYPE_ED)
        zb_zdo_pim_set_long_poll_interval(poll_interval); // should be possible to change durint runtime too, if needed
}
void return_started(){
    start_response.pan_id = zb_get_pan_id();
    zb_get_extended_pan_id(&start_response.ext_pan_id);
    callbacks.zboss_start(start_response);
}
void started(zb_ret_t ret){
    ZB_BZERO(&start_response,sizeof(start_resp_t));
    start_response.status = ret;
    if(ret != 0)
    {
        callbacks.zboss_start(start_response);
        return;
    }

    ZB_SCHEDULE_APP_CALLBACK(setpolicy,0);
    start_response.channel = zb_get_current_channel();
    start_response.addr = zb_get_short_address();
    ZB_BZERO(start_response.nwk_key,48);
    ncp_host_state_get_nwk_key(start_response.nwk_key,0);
    ncp_host_state_get_nwk_key(start_response.nwk_key + 16,1);
    ncp_host_state_get_nwk_key(start_response.nwk_key + 32,2);
    ncp_host_get_zigbee_pan_id();
    ncp_host_get_extended_pan_id();
    ncp_host_set_tx_power(8); // set default
    ZB_SCHEDULE_APP_ALARM(return_started,0,ZB_MILLISECONDS_TO_BEACON_INTERVAL(200)); // maybe delay could be less
}

void stopfunction(zb_uint8_t asd)
{
    TRACE_MSG(TRACE_APP1, "shut started", (FMT__0));
    zb_scheduler_start_shutting();
    TRACE_MSG(TRACE_APP1, "serial deinit", (FMT__0));
    zb_osif_serial_deinit();
    TRACE_MSG(TRACE_APP1, "sched stopping", (FMT__0));
    zb_sched_stop();
    free(local_ic);
    TRACE_MSG(TRACE_APP1, "shut finished", (FMT__0));
}
void stop()
{
    ZB_SCHEDULE_APP_CALLBACK(stopfunction,0);
}

void zboss_signal_handler(zb_uint8_t param)
{
    zb_zdo_app_signal_hdr_t *sg_p = NULL;
    /* Get application signal from the buffer */
    zb_zdo_app_signal_type_t sig = zb_get_app_signal(param, &sg_p);

    if (ZB_GET_APP_SIGNAL_STATUS(param) == 0)
    {
        switch(sig)
        {
            case ZB_ZDO_SIGNAL_SKIP_STARTUP:
                break;
            case ZB_ZDO_SIGNAL_DEFAULT_START:
            case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
            case ZB_BDB_SIGNAL_DEVICE_REBOOT:
                TRACE_MSG(TRACE_APP1, "Device STARTED OK", (FMT__0));
                zb_zcl_set_backward_comp_mode(ZB_ZCL_AUTO_MODE);
                zb_zcl_set_backward_compatible_statuses_mode(ZB_ZCL_STATUSES_ZCL8_MODE);
                bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
                break;

            case ZB_BDB_SIGNAL_STEERING:
                TRACE_MSG(TRACE_APP1, "Successfull steering", (FMT__0));
                started(0);
                break;

            case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
                zb_zdo_signal_device_annce_params_t *dev_annce_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p, zb_zdo_signal_device_annce_params_t);
                TRACE_MSG(TRACE_APP1, "Device annce, address: %x", (FMT__H, dev_annce_params->device_short_addr));
                callbacks.device_annce(*dev_annce_params);
                break;         

            case ZB_ZDO_SIGNAL_LEAVE_INDICATION:
                TRACE_MSG(TRACE_APP1, "Device leave", (FMT__0));
                zb_zdo_signal_leave_indication_params_t *dev_leave_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p, zb_zdo_signal_leave_indication_params_t);
                callbacks.device_leave_indication(*dev_leave_params);
                break;  

            case ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
                TRACE_MSG(TRACE_APP1, "Loading application production config", (FMT__0));
                break;

            case ZB_ZDO_SIGNAL_LEAVE:
                zb_zdo_signal_leave_params_t *leave_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p, zb_zdo_signal_leave_params_t);
                TRACE_MSG(TRACE_APP1, "Leave signal, type %d", (FMT__H, leave_params->leave_type));
                if(leave_params->leave_type == ZB_NWK_LEAVE_TYPE_RESET)
                    callbacks.leave_signal();
                break;

            default:
                TRACE_MSG(TRACE_ERROR, "Unknown signal %hd", (FMT__H, sig));
        }
    }
    else if (sig == ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY)
    {
        TRACE_MSG(TRACE_APP1, "Production config is not present or invalid", (FMT__0));
    }
    else
    {
        TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, ZB_GET_APP_SIGNAL_STATUS(param)));
        started(ZB_GET_APP_SIGNAL_STATUS(param));
    }

    /* Free the buffer if it is not used */
    if (param)
    {
        zb_buf_free(param);
    }
}
uint8_t device_handler(uint8_t param)
{
    zb_uint8_t success = 0;
    zb_zcl_parsed_hdr_t *cmd_info;
    cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
    if(cmd_info->is_common_command)
    {
        switch (cmd_info->cmd_id)
        {
        case ZB_ZCL_CMD_READ_ATTRIB_RESP:
            zcl_read_attr_resp(param);
            success = 1;
            break;
        case ZB_ZCL_CMD_WRITE_ATTRIB_RESP:
            zcl_write_attr_resp(param);
            success = 1;
            break;
        case ZB_ZCL_CMD_DISC_ATTRIB_RESP:
            zcl_disc_attr_resp(param);
            success = 1;
            break;        
        case ZB_ZCL_CMD_CONFIG_REPORT_RESP:
            configure_reporting_resp(param);
            success = 1;
            break;
        case ZB_ZCL_CMD_READ_REPORT_CFG_RESP:
            read_reporting_resp(param);
            success = 1;
            break;
        case ZB_ZCL_CMD_DISCOVER_COMMANDS_RECEIVED_RES:
            discover_commands_resp(param);
            success = 1;
            break;
        default:
            break;
        }
    }
    if(success)
        zb_zcl_send_default_resp_ext(param,cmd_info,ZB_ZCL_STATUS_SUCCESS);
    else
    {
        if(!cmd_info->is_common_command)
        {
            TRACE_MSG(TRACE_APP1, "Command received: ep_id %hd, cluster_id %d, cmd_id %hd is_common_command %hd, disable_default_response %hd, is_manuf_specific %hd", 
            (FMT__H_D_H_H_H_H, cmd_info->addr_data.common_data.dst_endpoint, cmd_info->cluster_id, cmd_info->cmd_id, ZB_B2U(cmd_info->is_common_command), ZB_B2U(cmd_info->disable_default_response), ZB_B2U(cmd_info->is_manuf_specific)));
            zcl_cmd_info_t info;
            info.source = cmd_info->addr_data.common_data.source.u.short_addr;
            info.ep = cmd_info->addr_data.common_data.dst_endpoint;
            info.cluster = cmd_info->cluster_id;
            info.cmd = cmd_info->cmd_id;
            info.disable_default_response = cmd_info->disable_default_response;
            info.is_manuf_specific = cmd_info->is_manuf_specific;
            info.payload_length = zb_buf_len(param);
            info.payload = (zb_uint8_t*)zb_buf_begin(param);
            callbacks.cmd_receive(info);
        }           
    }
    return success;
}


// permit joining for 180s
void mgmt_permit_joining_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP2, "mgmt_permit_joining_req_cb called", (FMT__0));
    zb_zdo_mgmt_permit_joining_resp_t *resp;
    resp = (zb_zdo_mgmt_permit_joining_resp_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP1, "mgmt_permit_joining_req_cb status: %hd", (FMT__H, resp->status));
    callbacks.mgmt_permit_joining_req(resp->status);
    zb_buf_free(param);
}
void mgmt_permit_joining_req_p(zb_uint8_t param)
{
    zb_zdo_mgmt_permit_joining_req_param_t *req_param = ZB_BUF_GET_PARAM(param, zb_zdo_mgmt_permit_joining_req_param_t);
    ZB_BZERO(req_param, sizeof(zb_zdo_mgmt_permit_joining_req_param_t));
    req_param->dest_addr = zb_get_short_address();
    req_param->permit_duration = ZB_BDBC_MIN_COMMISSIONING_TIME_S;
    req_param->tc_significance = ZB_TRUE;
    zb_zdo_mgmt_permit_joining_req(param, mgmt_permit_joining_req_cb);
}
void mgmt_permit_joining_req()
{
    TRACE_MSG(TRACE_APP2, "mgmt_permit_joining_req called", (FMT__0));
    zb_buf_get_out_delayed(mgmt_permit_joining_req_p);
}

// add ic
void secur_ic_add_cb(zb_ret_t status)
{
    TRACE_MSG(TRACE_APP1, "secur_ic_add_cb status: %hd", (FMT__H, status));
    free(add_ic);
    add_ic = NULL;
    callbacks.secur_ic_add(status);
}
void secur_ic_add_p(zb_uint8_t param)
{
    zb_uint8_t size = zb_ic_size_by_type[secur_ic_add_req_par.ic_type];
    add_ic = malloc(size + 2);
    memcpy(add_ic,secur_ic_add_req_par.ic,size);
    zb_uint16_t crc = ~zb_crc16(secur_ic_add_req_par.ic, 0xffff, size);
    memcpy(add_ic + size, &crc, 2);
    zb_uint8_t add_ieee[8];
    memcpy(add_ieee, &secur_ic_add_req_par.ieee_addr, 8);
    zb_secur_ic_add(add_ieee, secur_ic_add_req_par.ic_type, add_ic, secur_ic_add_cb);
}
void secur_ic_add(install_code_t req)
{
    TRACE_MSG(TRACE_APP2, "secur_ic_add called", (FMT__0));
    secur_ic_add_req_par = req;
    ZB_SCHEDULE_APP_CALLBACK(secur_ic_add_p,0);
}

// list all ics
void secur_ic_list_cb(zb_uint8_t param)
{
    zb_secur_ic_get_list_resp_t *resp;
    resp = ZB_BUF_GET_PARAM(param, zb_secur_ic_get_list_resp_t);
    TRACE_MSG(TRACE_APP1, "secur_ic_list_cb status: %hd", (FMT__H, resp->status));
    zb_aps_installcode_nvram_t *list = (zb_aps_installcode_nvram_t*)zb_buf_begin(param);
    secur_ic_get_list_resp_t response;
    ZB_BZERO(&response,sizeof(secur_ic_get_list_resp_t));
    response.status = resp->status;
    if(resp->status == 0)
    {
        response.count = resp->ic_table_list_count;
        response.entries = resp->ic_table_entries;
        if(response.count > 0)
            response.list = list;
    }
    callbacks.secur_ic_list(response);
    zb_buf_free(param);
}
void secur_ic_list_p(zb_uint8_t param, zb_uint16_t index)
{
    zb_secur_ic_get_list_req_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_secur_ic_get_list_req_t);
    req->response_cb = secur_ic_list_cb;
    req->start_index = (zb_uint8_t)index;
    zb_secur_ic_get_list_req(param);
}
void secur_ic_list(zb_uint16_t index)
{
    TRACE_MSG(TRACE_APP2, "secur_ic_list called", (FMT__0));
    zb_buf_get_out_delayed_ext(secur_ic_list_p,index,0);
}

// remove all ics
void secur_ic_remove_all_cb(zb_uint8_t param)
{
    zb_secur_ic_remove_all_resp_t *resp;
    resp = ZB_BUF_GET_PARAM(param, zb_secur_ic_remove_all_resp_t);
    TRACE_MSG(TRACE_APP1, "secur_ic_remove_all status: %hd", (FMT__H, resp->status));
    callbacks.secur_ic_remove_all(resp->status);
    zb_buf_free(param);
}
void secur_ic_remove_all_p(zb_uint8_t param)
{
    zb_secur_ic_remove_all_req_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_secur_ic_remove_all_req_t);
    req->response_cb = secur_ic_remove_all_cb;
    zb_secur_ic_remove_all_req(param);
}
void secur_ic_remove_all()
{
    TRACE_MSG(TRACE_APP2, "secur_ic_remove_all called", (FMT__0));
    zb_buf_get_out_delayed(secur_ic_remove_all_p);
}

// leave request
void mgmt_leave_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP2, "mgmt_leave_req_cb called", (FMT__0));
    zb_zdo_mgmt_leave_res_t *resp;
    resp = (zb_zdo_mgmt_leave_res_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP1, "mgmt_leave_req_cb status: %hd", (FMT__H, resp->status));
    callbacks.mgmt_leave_req(resp->status);
    zb_buf_free(param);
}
void mgmt_leave_req_p(zb_uint8_t param)
{
    zb_zdo_mgmt_leave_param_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_zdo_mgmt_leave_param_t);
    ZB_BZERO(req,sizeof(zb_zdo_mgmt_leave_param_t));
    req->dst_addr = mgmt_leave_req_par.address;
    req->rejoin = mgmt_leave_req_par.rejoin;
    req->remove_children = mgmt_leave_req_par.remove_children;
    zdo_mgmt_leave_req(param, mgmt_leave_req_cb);
}
void mgmt_leave_req(mgmt_leave_req_t req)
{
    TRACE_MSG(TRACE_APP2, "mgmt_leave_req called", (FMT__0));
    mgmt_leave_req_par = req;
    zb_buf_get_out_delayed(mgmt_leave_req_p);
}

// mgmt_lqi_req
void mgmt_lqi_req_cb(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_zdo_mgmt_lqi_resp_t *resp = (zb_zdo_mgmt_lqi_resp_t*)zb_buf_begin(buf);
    TRACE_MSG(TRACE_APP1, "mgmt_lqi_req_cb Status: %d", (FMT__H, resp->status));
    TRACE_MSG(TRACE_APP2, "mgmt_lqi_req_cb Count: %d", (FMT__H, resp->neighbor_table_list_count));
    TRACE_MSG(TRACE_APP2, "mgmt_lqi_req_cb Entries: %d", (FMT__H, resp->neighbor_table_entries));
    zb_zdo_neighbor_table_record_t *record = (zb_zdo_neighbor_table_record_t*)(resp + 1);
    if(resp->neighbor_table_list_count > 0)
        TRACE_MSG(TRACE_APP2, "mgmt_lqi_req_cb addr: %d", (FMT__D, record->network_addr));
    mgmt_lqi_resp_t response = {.status = resp->status, .neighbor_table_entries =resp->neighbor_table_entries, .neighbor_table_list_count = resp->neighbor_table_list_count,.list = record};
    callbacks.mgmt_lqi_req(response);
    zb_buf_free(buf);
}
void mgmt_lqi_req_p(zb_uint8_t param)
{
    zb_zdo_mgmt_lqi_param_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_zdo_mgmt_lqi_param_t);
    req->dst_addr = address_index_lqi.address;
    req->start_index = address_index_lqi.index;
    zb_zdo_mgmt_lqi_req(param, mgmt_lqi_req_cb);
}
void mgmt_lqi_req(address_index_t settings){ 
    TRACE_MSG(TRACE_APP2, "mgmt_lqi_req addr: %d", (FMT__D, settings.address));
    address_index_lqi = settings;
    zb_buf_get_out_delayed(mgmt_lqi_req_p);
}

// nwk addr req
void nwk_addr_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP2, "nwk_addr_req_cb called", (FMT__0));
    zb_zdo_nwk_addr_resp_head_t *resp;
    resp = (zb_zdo_nwk_addr_resp_head_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP1, "nwk_addr_req_cb status: %hd", (FMT__H, resp->status));
    TRACE_MSG(TRACE_APP2, "nwk_addr_req_cb short: %hd", (FMT__H, resp->nwk_addr));
    callbacks.nwk_addr_req(*resp);
    zb_buf_free(param);
}
void nwk_addr_req_p(zb_uint8_t param)
{
    zb_zdo_nwk_addr_req_param_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_zdo_nwk_addr_req_param_t);
    memcpy(req,&nwk_addr_req_par,sizeof(zb_zdo_nwk_addr_req_param_t));
    zb_zdo_nwk_addr_req(param, nwk_addr_req_cb);
}
void nwk_addr_req(zb_zdo_nwk_addr_req_param_t req)
{
    TRACE_MSG(TRACE_APP2, "nwk_addr_req called", (FMT__0));
    nwk_addr_req_par = req;
    zb_buf_get_out_delayed(nwk_addr_req_p);
}

// active ep request
void active_ep_req_cb(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)zb_buf_begin(buf);
    TRACE_MSG(TRACE_APP1, "active_ep_req_cb Status: %d", (FMT__H, resp->status));
    TRACE_MSG(TRACE_APP2, "active_ep_req_cb Ep count: %d", (FMT__H, resp->ep_count));
    zb_uint8_t *ep = (zb_uint8_t*)(resp + 1);
    ep_resp_t response = {.response = *resp, .array = ep};
    callbacks.active_ep_req(response);
    zb_buf_free(buf);
}
void active_ep_req_p(zb_uint8_t param, zb_uint16_t addr)
{
    zb_bufid_t buf = param;
    zb_zdo_active_ep_req_t *req;
    req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_active_ep_req_t));
    req->nwk_addr = addr;
    zb_zdo_active_ep_req(param, active_ep_req_cb);
}
void active_ep_req(zb_uint16_t addr){ 
    TRACE_MSG(TRACE_APP2, "active_ep_req called", (FMT__0));
    zb_buf_get_out_delayed_ext(active_ep_req_p, addr, 0);
}

// diagnostic data
get_diag_data_cb(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_zdo_get_diag_data_resp_params_t *resp = ZB_BUF_GET_PARAM(buf, zb_zdo_get_diag_data_resp_params_t);
    callbacks.get_diag_data(*resp);
    zb_buf_free(param);
}
void get_diag_data_p(zb_uint8_t param, zb_uint16_t addr)
{
    zb_bufid_t buf = param;
    zb_zdo_get_diag_data_req_params_t *req;
    req = ZB_BUF_GET_PARAM(buf, zb_zdo_get_diag_data_req_params_t);
    req->short_address = addr;
    zb_zdo_get_diag_data_async(param, get_diag_data_cb);
}
void get_diag_data(zb_uint16_t addr){ 
    TRACE_MSG(TRACE_APP1, "get_diag_data called", (FMT__0));        
    zb_buf_get_out_delayed_ext(get_diag_data_p, addr, 0);
}

// simple descriptor request
void simple_desc_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP1, "simple_desc_req_cb called", (FMT__0));
    zb_bufid_t buf = param;
    zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)zb_buf_begin(buf);
    simple_desc_t desc = {
        .status = resp->hdr.status,
        .nwk_addr = resp->hdr.nwk_addr,
        .length = resp->hdr.length,
        .endpoint = resp->simple_desc.endpoint,
        .app_profile_id = resp->simple_desc.app_profile_id,
        .app_device_id = resp->simple_desc.app_device_id,
        .app_device_version = resp->simple_desc.app_device_version,
        .app_input_cluster_count = resp->simple_desc.app_input_cluster_count,
        .app_output_cluster_count = resp->simple_desc.app_output_cluster_count,
        .app_cluster_list = resp->simple_desc.app_cluster_list
    };
    callbacks.simple_desc_req(desc);
    zb_buf_free(buf);
}
void simple_desc_req_p(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_zdo_simple_desc_req_t *req;
    req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_simple_desc_req_t));
    memcpy(req,&simple_desc_req_par,sizeof(zb_zdo_simple_desc_req_t));
    zb_zdo_simple_desc_req(param, simple_desc_req_cb);
}
void simple_desc_req(zb_zdo_simple_desc_req_t settings)
{
    TRACE_MSG(TRACE_APP2, "simple_desc_req ep: %d", (FMT__H, settings.endpoint));
    simple_desc_req_par = settings;
    zb_buf_get_out_delayed(simple_desc_req_p);
}

// ZCL command
void zcl_send_cmd_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *cmd_send_status = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP1, "zcl_send_cmd_cb status: %d", (FMT__H, cmd_send_status->status));
    callbacks.zcl_send_cmd(cmd_send_status->status);
    zb_buf_free(param);
}
void zcl_send_cmd_p(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_uint8_t* ptr = ZB_ZCL_START_PACKET(buf);                                        \
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL_A(ptr, zcl_cmd_req_par.direction, zcl_cmd_req_par.is_manuf_specific, zcl_cmd_req_par.disable_default_response);            \
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(ptr, ZB_ZCL_GET_SEQ_NUM(),  zcl_cmd_req_par.is_manuf_specific, zcl_cmd_req_par.manuf_code, zcl_cmd_req_par.command);           \
    if(zcl_cmd_req_par.size > 0)
        ZB_ZCL_PACKET_PUT_DATA_N(ptr,zcl_cmd_req_par.data,zcl_cmd_req_par.size);
    ZB_ZCL_FINISH_PACKET(buf, ptr)                                                     \
    ZB_ZCL_SEND_COMMAND_SHORT(                                                            \
      buf, zcl_cmd_req_par.nwk_addr, zcl_cmd_req_par.dst_addr_mode, zcl_cmd_req_par.dst_ep, zcl_cmd_req_par.src_ep, zcl_cmd_req_par.profile_id, zcl_cmd_req_par.cluster_id, zcl_send_cmd_cb);
}
void zcl_send_cmd(zcl_cmd_req_t settings)
{
    TRACE_MSG(TRACE_APP2, "zcl_send_cmd called", (FMT__0));
    zcl_cmd_req_par = settings;
    zb_buf_get_out_delayed(zcl_send_cmd_p);
}

// read attributes
void zcl_read_attr_resp(zb_uint8_t param){
    zb_zcl_read_attr_res_t *resp;
    ZB_ZCL_GENERAL_GET_NEXT_READ_ATTR_RES(param, resp);
    if(resp != NULL)
    {
        TRACE_MSG(TRACE_APP1, "zcl_read_attr_resp status: %hd", (FMT__H, resp->status));
        if(resp->attr_id != zcl_read_attr_req_par.attribute)
        {
            TRACE_MSG(TRACE_ERROR, "zcl_read_attr_resp attribute id mismatch, expected %d, got %d", (FMT__D_D, zcl_read_attr_req_par.attribute, resp->attr_id));
            return;
        }
        zcl_read_attr_resp_t response;
        response.status = resp->status;
        response.attr_id = resp->attr_id;
        response.attr_type = resp->attr_type;
        response.attr_length = zb_zcl_get_attribute_size(resp->attr_type,resp->attr_value);
        response.array = resp->attr_value;
        callbacks.attr_read(response);
    }
    //device handler frees buf
}
void zcl_read_attr_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "zcl_read_attr_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void zcl_read_attr_p(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_uint8_t* ptr;
    if(zcl_read_attr_req_par.is_manuf_specific) {
        ZB_ZCL_GENERAL_INIT_READ_ATTR_REQ_MANUF((buf), ptr, ZB_ZCL_FRAME_DIRECTION_TO_SRV,zcl_read_attr_req_par.disable_default_response,zcl_read_attr_req_par.manuf_code);
    }
    else {
        ZB_ZCL_GENERAL_INIT_READ_ATTR_REQ((buf), ptr, zcl_read_attr_req_par.disable_default_response);
    }
    ZB_ZCL_GENERAL_ADD_ID_READ_ATTR_REQ(ptr, (zcl_read_attr_req_par.attribute));
    ZB_ZCL_GENERAL_SEND_READ_ATTR_REQ((buf), ptr, (zcl_read_attr_req_par.nwk_addr), (zcl_read_attr_req_par.dst_addr_mode), (zcl_read_attr_req_par.dst_ep),
                                   (zcl_read_attr_req_par.src_ep), (zcl_read_attr_req_par.profile_id),
                                   (zcl_read_attr_req_par.cluster_id), zcl_read_attr_cb);
}
void zcl_read_attr(zcl_read_attr_req_t settings)
{
    TRACE_MSG(TRACE_APP2, "zcl_read_attr called", (FMT__0));
    zcl_read_attr_req_par = settings;
    zb_buf_get_out_delayed(zcl_read_attr_p);
}


// write attributes
void zcl_write_attr_resp(zb_uint8_t param){
    zb_zcl_write_attr_res_t *resp;
    ZB_ZCL_GET_NEXT_WRITE_ATTR_RES(param, resp);
    // in a successful response, attribute identifiers are not included. If at least one write is unsuccessful, that is when the response follows this format (ZCL 2.5.5.1.2)
    // on success, attr_id will contain memory junk
    if(resp != NULL)
        callbacks.attr_write(resp->status);
    else
        callbacks.attr_write(-1);
    //device handler frees buf
}
void zcl_write_attr_cb(zb_uint8_t param)
{     
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "zcl_write_attr_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void zcl_write_attr_p(zb_uint8_t param)
{
    zb_uint8_t *cmd_ptr;
    if(zcl_write_attr_req_par.is_manuf_specific) {
        ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ_MANUF((param), cmd_ptr, ZB_ZCL_FRAME_DIRECTION_TO_SRV, zcl_write_attr_req_par.disable_default_response, zcl_write_attr_req_par.manuf_code);
    }
    else {
        ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ((param), cmd_ptr, zcl_write_attr_req_par.disable_default_response);
    }
    ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(cmd_ptr, (zcl_write_attr_req_par.attribute), (zcl_write_attr_req_par.attr_type), (zcl_write_attr_req_par.data));
    ZB_ZCL_GENERAL_SEND_WRITE_ATTR_REQ((param), cmd_ptr, zcl_write_attr_req_par.nwk_addr, zcl_write_attr_req_par.dst_addr_mode,
    zcl_write_attr_req_par.dst_ep, zcl_write_attr_req_par.src_ep, zcl_write_attr_req_par.profile_id, (zcl_write_attr_req_par.cluster_id), zcl_write_attr_cb);
}
void zcl_write_attr(zcl_write_attr_req_t settings)
{
    TRACE_MSG(TRACE_APP2, "zcl_write_attr called", (FMT__0));
    zcl_write_attr_req_par = settings;
    zb_buf_get_out_delayed(zcl_write_attr_p);
}

// attribute discovery
void zcl_disc_attr_resp(zb_uint8_t param)
{     
    zcl_disc_attr_resp_t response;
    ZB_ZCL_GENERAL_GET_COMPLETE_DISC_RES(param,response.complete);
    TRACE_MSG(TRACE_APP2, "zcl_disc_attr_resp complete: %d", (FMT__H, response.complete));
    response.length = zb_buf_len(param)/sizeof(zb_zcl_disc_attr_info_t);
    TRACE_MSG(TRACE_APP2, "zcl_disc_attr_resp len: %d", (FMT__H, response.length));
    if(response.length > 0)
        response.list = (zb_zcl_disc_attr_info_t*)zb_buf_begin(param);
    else 
        response.list = NULL;
    callbacks.attr_disc(response);
    //device handler frees buf
}
void zcl_disc_attr_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "zcl_disc_attr_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void zcl_disc_attr_p(zb_uint8_t param)
{
    if(zcl_disc_attr_req_par.is_manuf_specific) {
        ZB_ZCL_GENERAL_DISC_ATTR_REQ_MANUF(param, cmd_ptr,
            ZB_ZCL_FRAME_DIRECTION_TO_SRV,
            zcl_disc_attr_req_par.disable_default_response,
            zcl_disc_attr_req_par.manuf_code,
            zcl_disc_attr_req_par.start_attr_id,
            zcl_disc_attr_req_par.max,
            zcl_disc_attr_req_par.nwk_addr,
            zcl_disc_attr_req_par.dst_addr_mode,
            zcl_disc_attr_req_par.dst_ep,
            zcl_disc_attr_req_par.src_ep,
            zcl_disc_attr_req_par.profile_id,
            zcl_disc_attr_req_par.cluster_id,
            zcl_disc_attr_cb
        );
    }
    else {
        ZB_ZCL_GENERAL_DISC_READ_ATTR_REQ(param,
            zcl_disc_attr_req_par.disable_default_response,
            zcl_disc_attr_req_par.start_attr_id,
            zcl_disc_attr_req_par.max,
            zcl_disc_attr_req_par.nwk_addr,
            zcl_disc_attr_req_par.dst_addr_mode,
            zcl_disc_attr_req_par.dst_ep,
            zcl_disc_attr_req_par.src_ep,
            zcl_disc_attr_req_par.profile_id,
            zcl_disc_attr_req_par.cluster_id,
            zcl_disc_attr_cb
        );
    }
}
void zcl_disc_attr(zcl_disc_attr_req_t settings)
{
    TRACE_MSG(TRACE_APP2, "zcl_disc_attr called", (FMT__0));
    zcl_disc_attr_req_par = settings;
    zb_buf_get_out_delayed(zcl_disc_attr_p);
}

// bind request
void bind_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP1, "bind_req_cb called", (FMT__0));
    zb_zdo_bind_resp_t *resp = (zb_zdo_bind_resp_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP2, "bind_req_cb status: %d", (FMT__H, resp->status));
    callbacks.bind_req(resp->status);
    zb_buf_free(param);
}
void bind_req_p(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP2, "bind_req_p", (FMT__0));
    zb_zdo_bind_req_param_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_zdo_bind_req_param_t);
    TRACE_MSG(TRACE_APP2, "bind_req_p 2", (FMT__0));
    ZB_MEMCPY(req, &bind_req_par, sizeof(zb_zdo_bind_req_param_t));
    zb_zdo_bind_req(param, bind_req_cb);
}
void bind_req(zb_zdo_bind_req_param_t settings)
{
    TRACE_MSG(TRACE_APP2, "bind_req", (FMT__0));
    bind_req_par = settings;
    zb_buf_get_out_delayed(bind_req_p);
}
// unbind request
void unbind_req_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP1, "unbind_req_cb called", (FMT__0));
    zb_zdo_bind_resp_t *resp = (zb_zdo_bind_resp_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP2, "unbind_req_cb status: %d", (FMT__H, resp->status));
    callbacks.unbind_req(resp->status);
    zb_buf_free(param);
}
void unbind_req_p(zb_uint8_t param)
{
    zb_zdo_bind_req_param_t *req;
    req = ZB_BUF_GET_PARAM(param, zb_zdo_bind_req_param_t);
    ZB_MEMCPY(req, &bind_req_par, sizeof(zb_zdo_bind_req_param_t));
    zb_zdo_unbind_req(param, unbind_req_cb);
}
void unbind_req(zb_zdo_bind_req_param_t settings)
{
    TRACE_MSG(TRACE_APP2, "unbind_req", (FMT__0));
    bind_req_par = settings;
    zb_buf_get_out_delayed(unbind_req_p);
}

// get binding table
void binding_table_cb(zb_uint8_t param)
{
    TRACE_MSG(TRACE_APP2, "binding_table_cb called", (FMT__0));
    zb_zdo_mgmt_bind_resp_t *resp = (zb_zdo_mgmt_bind_resp_t*)zb_buf_begin(param);
    TRACE_MSG(TRACE_APP2, "binding_table_cb entries: %d", (FMT__H, resp->binding_table_entries));
    TRACE_MSG(TRACE_APP2, "binding_table_cb start: %d", (FMT__H, resp->start_index));
    binding_table_resp_t response;
    ZB_BZERO(&response, sizeof(binding_table_resp_t));
    response.status = resp->status;
    if(resp->status == 0)
    {
        response.count = resp->binding_table_list_count;
        response.entries = resp->binding_table_entries;
        if(resp->binding_table_list_count > 0)
            response.list = resp->records;
    }
    callbacks.binding_table(response);
    zb_buf_free(param);
}
void binding_table_p(zb_uint8_t param)
{
    zb_bufid_t buf = param;
    zb_zdo_mgmt_bind_param_t *req;
    req = ZB_BUF_GET_PARAM(buf, zb_zdo_mgmt_bind_param_t);
    req->start_index = address_index_bindingtable.index;
    req->dst_addr = address_index_bindingtable.address;
    zb_zdo_mgmt_bind_req(param, binding_table_cb);
}
void binding_table(address_index_t settings){ 
    TRACE_MSG(TRACE_APP2, "binding_table called", (FMT__0));
    address_index_bindingtable = settings;
    zb_buf_get_out_delayed(binding_table_p);
}
// configure reporting
void configure_reporting_resp(zb_uint8_t param)
{
    zb_zcl_configure_reporting_res_t *response;
    ZB_ZCL_GENERAL_GET_NEXT_CONFIGURE_REPORTING_RES(param,response);
    if(response != NULL)
        callbacks.configure_reporting(response->status);
    else
        callbacks.configure_reporting(-1);
    //device handler frees buf
}
void configure_reporting_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "configure_reporting_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void configure_reporting_p(zb_uint8_t param)
{
    zb_uint8_t *cmd_ptr;
    if(configure_reporting_par.is_manuf_specific)
    {
        ZB_ZCL_GENERAL_INIT_CONFIGURE_REPORTING_SRV_REQ_MANUF(param, cmd_ptr, configure_reporting_par.disable_default_response,configure_reporting_par.manuf_code);
    }
    else
    {
        ZB_ZCL_GENERAL_INIT_CONFIGURE_REPORTING_SRV_REQ(param, cmd_ptr, configure_reporting_par.disable_default_response);
    }
    zcl_reporting_configuration_t *conf = &configure_reporting_par.configuration;
    ZB_ZCL_GENERAL_ADD_SEND_REPORT_CONFIGURE_REPORTING_REQ(
        cmd_ptr, conf->attr_id, conf->attr_type, conf->min_interval, conf->max_interval, &(conf->change));
    ZB_ZCL_GENERAL_SEND_CONFIGURE_REPORTING_REQ(param, cmd_ptr, configure_reporting_par.nwk_addr, configure_reporting_par.dst_addr_mode,
        configure_reporting_par.dst_ep, configure_reporting_par.src_ep,
        configure_reporting_par.profile_id, configure_reporting_par.cluster_id, configure_reporting_cb);
}
void configure_reporting(zcl_configure_reporting_req_t settings){ 
    TRACE_MSG(TRACE_APP2, "configure_reporting called", (FMT__0));
    configure_reporting_par = settings;
    zb_buf_get_out_delayed(configure_reporting_p);
}

// read reporting
void read_reporting_resp(zb_uint8_t param)
{
    zcl_read_reporting_res_t response;
    ZB_BZERO(&response, sizeof(zcl_read_reporting_res_t));
    zb_zcl_read_reporting_cfg_rsp_t *res;
    ZB_ZCL_GENERAL_GET_NEXT_CONFIGURE_REPORTING_RES(param,res); 
    TRACE_MSG(TRACE_APP2, "read_reporting_resp status: %hd", (FMT__H, res->status));
    response.status = (res == NULL || res->direction == 1) ? 0xff : res->status;
    response.config.attr_id = res->attr_id;
    response.config.attr_type = res->u.clnt.attr_type;
    response.config.min_interval = res->u.clnt.min_interval;
    response.config.max_interval = res->u.clnt.max_interval;
    response.config.change = 0;
    if(zb_zcl_is_analog_data_type(res->u.clnt.attr_type))
    {
        memcpy(&response.config.change, res->u.clnt.delta,
                zb_zcl_get_attribute_size(res->u.clnt.attr_type, 0));
    }
    callbacks.read_reporting(response);
    //device handler frees buf
}
void read_reporting_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "read_reporting_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void read_reporting_p(zb_uint8_t param)
{
    zb_uint8_t *cmd_ptr;
    if(read_reporting_par.is_manuf_specific)
    {
        ZB_ZCL_GENERAL_INIT_READ_REPORTING_CONFIGURATION_SRV_REQ_MANUF(param, cmd_ptr, read_reporting_par.disable_default_response,read_reporting_par.manuf_code);
    }
    else
    {
        ZB_ZCL_GENERAL_INIT_READ_REPORTING_CONFIGURATION_SRV_REQ(param, cmd_ptr, read_reporting_par.disable_default_response);
    }
    ZB_ZCL_GENERAL_ADD_SEND_READ_REPORTING_CONFIGURATION_REQ(cmd_ptr,read_reporting_par.attribute)
    ZB_ZCL_GENERAL_SEND_READ_REPORTING_CONFIGURATION_REQ(param, cmd_ptr, read_reporting_par.nwk_addr, read_reporting_par.dst_addr_mode,
        read_reporting_par.dst_ep, read_reporting_par.src_ep,
        read_reporting_par.profile_id, read_reporting_par.cluster_id, read_reporting_cb);
}
void read_reporting(zcl_read_reporting_req_t settings){ 
    TRACE_MSG(TRACE_APP2, "read_reporting called", (FMT__0));
    read_reporting_par = settings;
    zb_buf_get_out_delayed(read_reporting_p);
}

// discover commands
void discover_commands_resp(zb_uint8_t param)
{
    zcl_discover_commands_resp_t resp;
    resp.length = zb_buf_len(param) - 1;
    zb_uint8_t *completed = zb_buf_begin(param);
    TRACE_MSG(TRACE_APP2, "discover_commands completed: %d", (FMT__H, *completed));
    if(resp.length != 0)
    {
        resp.list = zb_buf_begin(param) + sizeof(zb_uint8_t);
    }
    callbacks.discover_commands(resp);
    //device handler frees buf
}
void discover_commands_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *resp = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    TRACE_MSG(TRACE_APP2, "discover_commands_cb status: %d", (FMT__H, resp->status));
    zb_buf_free(param);
}
void discover_commands_p(zb_uint8_t param)
{
    ZB_ZCL_GENERAL_DISC_COMMAND_RECEIVED_REQ(param, ZB_ZCL_FRAME_DIRECTION_TO_SRV, discover_commands_par.disable_default_response, 
    discover_commands_par.nwk_addr, discover_commands_par.dst_addr_mode, discover_commands_par.dst_ep, discover_commands_par.src_ep,
    discover_commands_par.profile_id, discover_commands_par.cluster_id, discover_commands_cb, discover_commands_par.is_manuf_specific, 
    discover_commands_par.manuf_code, discover_commands_par.start_cmd_id, discover_commands_par.max_len);
}
void discover_commands(zcl_discover_commands_req_t settings){ 
    TRACE_MSG(TRACE_APP2, "discover_commands called", (FMT__0));
    discover_commands_par = settings;
    zb_buf_get_out_delayed(discover_commands_p);
}

void set_txpower_p(zb_uint8_t param,zb_uint16_t arg)
{
    zb_int8_t txpower = (zb_int8_t)arg;
    zb_ret_t ret = ncp_host_set_tx_power(txpower);
    TRACE_MSG(TRACE_APP2, "set txpower status: %d", (FMT__D, ret));
    callbacks.set_tx_power(ret);
}
void set_txpower(zb_int8_t txpower){ 
    TRACE_MSG(TRACE_APP2, "set_txpower called", (FMT__0));
    ZB_SCHEDULE_APP_CALLBACK2(set_txpower_p,0,(zb_uint16_t)txpower);
}

void get_txpower_2(zb_uint8_t param)
{
    zb_int8_t txpower = ncp_host_state_get_txpower();
    TRACE_MSG(TRACE_APP2, "get_txpower: %d", (FMT__H, txpower));
    callbacks.get_tx_power(txpower);
}
void get_txpower_1(zb_uint8_t param)
{
    zb_ret_t ret = ncp_host_get_tx_power();
    ZB_SCHEDULE_APP_ALARM(get_txpower_2,0,ZB_MILLISECONDS_TO_BEACON_INTERVAL(200));
}
void get_txpower(){ 
    TRACE_MSG(TRACE_APP2, "get_txpower called", (FMT__0));
    ZB_SCHEDULE_APP_CALLBACK(get_txpower_1,0);
}
