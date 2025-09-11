/*  Copyright (c)  DSR Corporation, Denver CO, USA.
PURPOSE: Linux mac transport general functions
*/

#define ZB_TRACE_FILE_ID 30019

#include "zb_config.h"

#if defined(ZB_MACSPLIT_HOST) || defined(ZB_MACSPLIT_DEVICE)

#include "zb_common.h"
#include "zb_macsplit_transport_linux.h"

#include <errno.h>

#if defined ZB_MAC_TRANSPORT_DUMP
static void zb_write_mcu_dump(zb_uint8_t* data, zb_uint_t len);
static void zb_write_host_dump(zb_uint8_t* data, zb_uint_t len);
#endif

/*
Note: that file is based on implementation from the linux embedded platform repo.
Updated to use routines from zb_osif_transport.c which initially designed for NCP only.
 */

void zb_linux_transport_init(void)
{
  TRACE_MSG(TRACE_MAC1, ">> zb_linux_transport_init, transport_type %hd",
    (FMT__H, MACSPLIT_CTX().transport_type));

#ifdef ZB_PLATFORM_LINUX
  zb_timer_enable_stop();
#endif

#if defined ZB_MAC_INTERFACE_SINGLE
  /* TODO: [Multi-MAC] currently OSIF transport is initialized during multi-mac platform initialization,
   * but some other configurations can exist, so review it */
  osif_transport_init();
#endif

  ZB_MACSPLIT_IOCTX().is_inited = ZB_TRUE;

  /*
    Now transport must regiter handler using osif_transport_add_io_handler().
    See osif_serial_common_init() as an example.
   */

  switch (MACSPLIT_CTX().transport_type)
  {
#if defined ZB_TRANSPORT_LINUX_UART || defined(ZB_SERIAL_PTY_MASTER)
  case ZB_MACSPLIT_TRANSPORT_TYPE_SERIAL:
    zb_mac_uart_open();
    break;
#endif /* defined ZB_TRANSPORT_LINUX_UART */
#if defined ZB_TRANSPORT_LINUX_SPI
  case ZB_MACSPLIT_TRANSPORT_TYPE_SPI:
    zb_linux_spi_init();
    break;
#endif /* defined ZB_TRANSPORT_LINUX_SPI */
  default:
      ZB_ASSERT(0);
  }

#if defined ZB_MAC_TRANSPORT_DUMP
  ZB_MACSPLIT_IOCTX().mcu_dump = fopen(zb_get_mcu_dump_path(), "w");
  ZB_MACSPLIT_IOCTX().host_dump = fopen(zb_get_host_dump_path(), "w");
  if (ZB_MACSPLIT_IOCTX().mcu_dump == NULL)
  {
    TRACE_MSG(TRACE_ERROR, "mcu dump fopen error", (FMT__0));
  }
  if (ZB_MACSPLIT_IOCTX().host_dump == NULL)
  {
    TRACE_MSG(TRACE_ERROR, "host dump fopen error", (FMT__0));
  }
#endif /* defined ZB_MAC_TRANSPORT_DUMP */
  TRACE_MSG(TRACE_MAC1, "<< zb_linux_transport_init", (FMT__0));
}


zb_bool_t zb_linux_transport_is_open()
{
  return ZB_MACSPLIT_IOCTX().is_inited;
}


#if 0

/* Change upper code (is it SGW?) to use osif_transport_add_io_handler() instead of that functions. */

/* Weak and stub functions for setting up and handling application transport data */
ZB_WEAK_PRE void zb_app_transport_setup_wait(osif_wait_control_t wait_control, zb_int_t *num) ZB_WEAK;
void zb_app_transport_setup_wait(osif_wait_control_t wait_control, zb_int_t *num)
{
  ZVUNUSED(wait_control);
  ZVUNUSED(num);

  /* Empty implementation. Should be defined by the application. */
}

ZB_WEAK_PRE zb_ret_t zb_app_transport_handle_data(osif_ipc_handle_t h, zb_uint8_t mask) ZB_WEAK;
zb_ret_t zb_app_transport_handle_data(osif_ipc_handle_t h, zb_uint8_t mask)
{
  ZVUNUSED(h);
  ZVUNUSED(mask);

  TRACE_MSG(TRACE_MAC3, "default zb_app_transport_handle_data for handler: %d", (FMT__D, h));

  /* Empty implementation. Should be defined by the application. */
  return RET_OK;
}
#endif


/**
 * Sends data, dump it to ZB_MAC_SPLIT_TO_MCU_DUMP_FILE
 *
 * @param buf - byte array with data to send
 * @param len - length of the array in bytes
 *
 * @return nothing
 */
void zb_linux_transport_send_data(zb_uint8_t* buf, zb_uint_t len)
{
  TRACE_MSG(TRACE_MAC2, ">zb_linux_transport_send_data", (FMT__0));
  switch (MACSPLIT_CTX().transport_type)
  {
#if defined ZB_TRANSPORT_LINUX_UART || defined(ZB_SERIAL_PTY_MASTER)
  case ZB_MACSPLIT_TRANSPORT_TYPE_SERIAL:
    zb_mac_uart_write(buf, len);
    break;
#endif /* ZB_TRANSPORT_LINUX_UART || ZB_SERIAL_PTY_MASTER */
#if defined ZB_TRANSPORT_LINUX_SPI
  case ZB_MACSPLIT_TRANSPORT_TYPE_SPI:
    ZB_ASSERT(SPI_CTX().is_inited);
    zb_linux_spi_command_write(buf, len);
    break;
#endif /* ZB_TRANSPORT_LINUX_SPI */
  default:
      ZB_ASSERT(0);
  }
#if defined ZB_MAC_TRANSPORT_DUMP
  zb_write_host_dump(buf, len);
#endif
  zb_osif_update_timer();
  TRACE_MSG(TRACE_MAC3, "<zb_linux_transport_send_data", (FMT__0));
}

/**
 * Checks if data are available in ZB_MACSPLIT_IOCTX().buffer
 *
 * @return if data are available
 */
zb_bool_t zb_linux_transport_is_data_available()
{
  return !ZB_RING_BUFFER_IS_EMPTY(&ZB_MACSPLIT_IOCTX().in_buffer);
}

/**
 * Gets next byte from ZB_MACSPLIT_IOCTX().buffer
 * Caller should check, if there is available data using zb_linux_transport_is_data_available()
 *
 * @return next byte
 */
zb_uint8_t zb_linux_transport_get_next_byte()
{
  zb_uint8_t ret = *ZB_RING_BUFFER_GET(&ZB_MACSPLIT_IOCTX().in_buffer);
  ZB_RING_BUFFER_FLUSH_GET(&ZB_MACSPLIT_IOCTX().in_buffer);
#ifdef ZB_MAC_TRANSPORT_DUMP
  zb_write_mcu_dump(&ret, 1);
#endif
  return ret;
}

#if defined ZB_MAC_TRANSPORT_DUMP
/**
 * Provide traffic dump file path from MCU
 */
ZB_WEAK_PRE const char* zb_get_mcu_dump_path() ZB_WEAK;
const char* zb_get_mcu_dump_path()
{
  return ZB_MAC_SPLIT_FROM_MCU_DUMP_FILE;
}

/**
 * Provide traffic dump file path to MCU
 */
ZB_WEAK_PRE const char* zb_get_host_dump_path() ZB_WEAK;
const char* zb_get_host_dump_path()
{
  return ZB_MAC_SPLIT_TO_MCU_DUMP_FILE;
}

static void zb_write_dump(FILE* f, zb_uint8_t* data, zb_uint_t len)
{
  zb_uint_t i;

  if (f)
  {
    for (i = 0; i < len; i++)
    {
      if (fprintf(f, "%02x ", data[i]) < 0)
      {
        TRACE_MSG(TRACE_ERROR, "dump write error: fprintf error", (FMT__0));
      }
    }
    fprintf(f, "\n");
    fflush(f);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "dump write error: file is not opened", (FMT__0));
  }
}

/**
 * Used for dump data received from MCU in MAC split case
 * Writes data to file specified in ZB_MAC_SPLIT_FROM_MCU_DUMP_FILE
 *
 * @param data - buffer with data to write
 * @param len - length of the buffer with data
 *
 * @return nothing
 */
static void zb_write_mcu_dump(zb_uint8_t* data, zb_uint_t len)
{
  zb_write_dump(ZB_MACSPLIT_IOCTX().mcu_dump, data, len);
}

/**
 * Used for dump data sent to MCU in MAC split case
 * Writes data to file specified in ZB_MAC_SPLIT_TO_MCU_DUMP_FILE
 *
 * @param data - buffer with data to write
 * @param len - length of the buffer with data
 *
 * @return nothing
 */
static void zb_write_host_dump(zb_uint8_t* data, zb_uint_t len)
{
  zb_write_dump(ZB_MACSPLIT_IOCTX().host_dump, data, len);
}
#endif /* defined ZB_MAC_TRANSPORT_DUMP */
#else
typedef int make_iso_compilers_happy;
#endif  /* ZB_MACSPLIT_HOST || ZB_MACSPLIT_DEVICE */
