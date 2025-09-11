#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

// Include the necessary header for your types
// Assuming "zboss_api.h" is already included as shown in your provided code

typedef struct start_params_s
{
  uint8_t port;
  uint8_t role;
  uint64_t ieee_addr;
  uint8_t nwk_key[16];
  uint32_t channel_mask;
} start_params_t;

typedef struct start_resp_s
{
  int status;
  uint16_t addr;
  uint8_t nwk_key[48]; // 3*16 bytes
  uint8_t channel;
  uint16_t pan_id;
  uint64_t ext_pan_id;
} start_resp_t;
// callbacks
typedef void (*log_cbt)(const char* str);
typedef void (*noparam_cbt)();
typedef void (*start_cbt)(start_resp_t resp);
/*
typedef void (*ret_cbt)(zb_ret_t resp);
typedef void (*get_diag_data_cbt)(zb_zdo_get_diag_data_resp_params_t resp);
typedef void (*active_ep_req_cbt)(ep_resp_t resp);
typedef void (*simple_desc_req_cbt)(simple_desc_t resp);
typedef void (*read_attr_cbt)(zcl_read_attr_resp_t resp);
typedef void (*device_annce_cbt)(zb_zdo_signal_device_annce_params_t resp);
typedef void (*device_leave_indication_cbt)(zb_zdo_signal_leave_indication_params_t resp);
*/

typedef struct __attribute__((packed)) callbacks_s{
  log_cbt log;
  start_cbt zboss_start;
  noparam_cbt leave_signal;
  noparam_cbt device_annce;
  noparam_cbt device_leave_indication;
  noparam_cbt mgmt_permit_joining_req;
  noparam_cbt mgmt_leave_req;
  noparam_cbt get_diag_data;
  noparam_cbt active_ep_req;
  noparam_cbt simple_desc_req;
  noparam_cbt zcl_send_cmd;
  noparam_cbt attr_read;
  noparam_cbt attr_write;
} callbacks_t;


// Define a function pointer type matching the `init` function signature
typedef void (*init_func_t)(callbacks_t list, start_params_t params);
typedef void (*start_func_t)();

void startcb(start_resp_t resp)
{
    printf("OK\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_shared_library>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *lib_path = argv[1];
    void *handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading shared library: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    // Clear existing errors
    dlerror();

    // Get the `init` function symbol
    init_func_t init_func = (init_func_t)dlsym(handle, "init");
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Error locating 'init' function: %s\n", error);
        dlclose(handle);
        return EXIT_FAILURE;
    }
    
    start_func_t start_func = (init_func_t)dlsym(handle, "start");
    error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Error locating 'start' function: %s\n", error);
        dlclose(handle);
        return EXIT_FAILURE;
    }

    // Prepare the arguments for `init`
    callbacks_t callbacks = {0}; // Zero-initialize; populate as needed
    callbacks.zboss_start = startcb;
    start_params_t params = {0};
    params.port = 1; // Example port
    params.role = 0; // Example role
    params.ieee_addr = 0x123456789ABCDEF0; // Example IEEE address
    memset(params.nwk_key, 0xAA, sizeof(params.nwk_key)); // Example network key
    params.channel_mask = 1 << 11; // Example channel mask

    // Call the `init` function
    init_func(callbacks, params);

    printf("Init function executed successfully.\n");

    start_func();
    
    // Close the shared library
    dlclose(handle);
    return EXIT_SUCCESS;
}