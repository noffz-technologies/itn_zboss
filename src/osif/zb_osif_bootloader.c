/*
 * Copyright (c) 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Commercial Usage
 * Licensees holding valid Commercial licenses may use
 * this file in accordance with the Commercial License
 * Agreement provided with the Software or, alternatively, in accordance
 * with the terms contained in a written agreement relevant to the usage of the file.
*/

#define ZB_TRACE_FILE_ID 30018
#include "zboss_api_core.h"
#include "zb_osif_bootloader.h"

/* MAC-split SoC for NS doesn't support bootloader */
#if !(defined ZB_NSNG && (defined ZB_MACSPLIT_DEVICE || defined ZB_MAC_TESTING_MODE))

zb_ret_t zb_osif_bootloader_run_after_reboot(void)
{
  zb_uint32_t magic_num = NCP_OTA_START_UPGRADE;
  return zb_osif_nvram_write_memory(ZBS_BL_META_ADDR, (zb_uint16_t)sizeof(magic_num), (zb_uint8_t *)&magic_num);
}


void zb_osif_bootloader_report_successful_loading(void)
{
  zb_ret_t ret;
  zb_uint32_t old_value = 0;
  zb_uint32_t reporting_value = NCP_OTA_IMAGE_IS_LOADED_SUCCESSFULLY;

  ret = zb_osif_nvram_read_memory(ZBS_BL_META_ADDR, (zb_uint16_t)sizeof(old_value), (zb_uint8_t*)&old_value);
  ZB_ASSERT(ret == RET_OK);

  if (old_value != NCP_OTA_IMAGE_IS_LOADED_SUCCESSFULLY)
  {
    ret = zb_osif_nvram_write_memory(ZBS_BL_META_ADDR, (zb_uint16_t)sizeof(reporting_value), (zb_uint8_t *)&reporting_value);
    ZB_ASSERT(ret == RET_OK);
  }
}

#endif /* !(ZB_MACSPLIT_DEVICE && ZB_NSNG) */