/*  Copyright (c)  DSR Corporation, Denver CO, USA.
PURPOSE: Linux UART API for the split MAC implementations
*/

#define ZB_TRACE_FILE_ID 30020

#include "zb_common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdio.h>

#if defined(ZB_TRANSPORT_LINUX_UART) || (defined(ZB_SERIAL_PTY_MASTER) && !defined(NCP_MODE))

#include "zb_macsplit_transport_linux.h"

/*
That code has interface of zb_macsplit_transport_linux_uart.c from linux embedded platform repo,
but its implementation uses routines from zb_osif_serial.c and zb_osif_serial_common.c
initially made for NCP.
*/


#ifdef ZB_MAC_TRANSPORT_UART_USE_RST_LINE
static zb_ret_t zb_mac_uart_open_rst_line()
{
  zb_uint8_t uart_path[ZB_MAC_TRANSPORT_UART_PATH_SIZE];

#if defined ZB_MAC_TRANSPORT_UART_PATH && defined ZB_MAC_TRANSPORT_UART_PATH_SUB_GHZ
#error Support for RST lines for dual UART MAC-split is not implemented yet
#endif /* defined ZB_MAC_TRANSPORT_UART_PATH && defined ZB_MAC_TRANSPORT_UART_PATH_SUB_GHZ */
  ZB_MACSPLIT_IOCTX().uart_rst_fd = -1;
  sprintf((char*)uart_path, "%s", zb_mac_transport_uart_rst_path_get());
  errno = 0;
  ZB_MACSPLIT_IOCTX().uart_rst_fd = open((const char *)uart_path, O_RDWR);

  if (ZB_MACSPLIT_IOCTX().uart_rst_fd < 0)
  {
    TRACE_MSG(TRACE_ERROR, "Unable to open uart rst line: %d", (FMT__D, errno));
    ZB_ASSERT(0);
  }
  return RET_OK;
}

/* Default implementation */
ZB_WEAK_PRE const zb_char_t* zb_mac_transport_uart_rst_path_get() ZB_WEAK;
const zb_char_t* zb_mac_transport_uart_rst_path_get()
{
  return ZB_MAC_TRANSPORT_UART_RST_PATH;
}

static zb_ret_t zb_mac_uart_close_rst_line()
{
  if (ZB_MACSPLIT_IOCTX().uart_rst_fd != -1)
  {
    close(ZB_MACSPLIT_IOCTX().uart_rst_fd);
    ZB_MACSPLIT_IOCTX().uart_rst_fd = -1;
  }
  return RET_OK;
}

static zb_ret_t zb_mac_uart_rst_radio()
{
  zb_ret_t ret = RET_OK;

  TRACE_MSG(TRACE_INFO1, ">zb_mac_uart_rst_radio %d", (FMT__D, ZB_MACSPLIT_IOCTX().uart_rst_fd));

  ZB_ASSERT(ZB_MACSPLIT_IOCTX().uart_rst_fd);

  do
  {
    /* clear gpio pin */
    if (write(ZB_MACSPLIT_IOCTX().uart_rst_fd, "0", 2) < 0)
    {
      zb_mac_uart_close_rst_line();
      TRACE_MSG(TRACE_ERROR, "Failed to clear reset gpio pin errno: %d", (FMT__D, errno));
      ret = RET_ERROR;
      break;
    }
    lseek(ZB_MACSPLIT_IOCTX().uart_rst_fd, 0, SEEK_SET);
    zb_osif_wait_ms(100);

    /* set gpio pin */
    if (write(ZB_MACSPLIT_IOCTX().uart_rst_fd, "1", 2) < 0)
    {
      zb_mac_uart_close_rst_line();
      TRACE_MSG(TRACE_ERROR, "Failed to set reset gpio pin errno: %d", (FMT__D, errno));
      ret = RET_ERROR;
      break;
    }
    lseek(ZB_MACSPLIT_IOCTX().uart_rst_fd, 0, SEEK_SET);
    zb_osif_wait_ms(100);
  } while (0);

  zb_macsplit_mlme_mark_radio_reset();

  TRACE_MSG(TRACE_INFO1, "<zb_mac_uart_rst_radio %hd", (FMT__H, ret));
  return ret;
}

zb_ret_t zb_linux_turn_off_radio()
{
  TRACE_MSG(TRACE_INFO1, "zb_linux_turn_off_radio %d", (FMT__D, ZB_MACSPLIT_IOCTX().uart_rst_fd));

  if (ZB_MACSPLIT_IOCTX().uart_rst_fd)
  {
    write(ZB_MACSPLIT_IOCTX().uart_rst_fd, "0", 2);
    return RET_OK;
  }
  return RET_ERROR;
}
#endif  /* ZB_MAC_TRANSPORT_UART_USE_RST_LINE */

#ifdef ZB_MACSPLIT_HOST
static void uart_rx_handler(zb_uint8_t b);
#endif /* ZB_MACSPLIT_HOST */

/**
 * Open file, defined in ZB_MAC_TRANSPORT_UART_PATH,
 * set com port parameters, if fail, invoke ZB_ASSERT(0)
 *
 * @return opened file descriptor
 */
void zb_mac_uart_open(void)
{
#ifdef ZB_MAC_TRANSPORT_UART_USE_RST_LINE
  zb_mac_uart_open_rst_line();
  zb_mac_uart_rst_radio();
#endif  /* ZB_MAC_TRANSPORT_UART_USE_RST_LINE */

  zb_osif_serial_init();
#if !defined ZB_HAVE_ASYNC_SERIAL
  #ifdef ZB_MACSPLIT_HOST
    zb_osif_set_uart_byte_received_cb(uart_rx_handler);
  #endif
#else
  #error Not implemented yet!
  /*
TODO: implement logic similar to

  read_data_len = ZB_RING_BUFFER_AVAILABLE_CONTINUOUS_PORTION(&ZB_MACSPLIT_IOCTX().in_buffer);
  read_data_len = zb_mac_uart_read(ZB_RING_BUFFER_PUT_RESERVE(&ZB_MACSPLIT_IOCTX().in_buffer), read_data_len);
  ZB_RING_BUFFER_FLUSH_BATCH_PUT(&ZB_MACSPLIT_IOCTX().in_buffer, read_data_len);
*/
#endif
}


#ifdef ZB_MACSPLIT_HOST
static void uart_rx_handler(zb_uint8_t b)
{
  if (!ZB_RING_BUFFER_IS_FULL(&ZB_MACSPLIT_IOCTX().in_buffer))
  {
    ZB_RING_BUFFER_PUT(&ZB_MACSPLIT_IOCTX().in_buffer, b);
  }
}
#endif


/**
 * Write data from buf to com port (file specified in ZB_MAC_TRANSPORT_UART_PATH)
 *
 * @param buf - buffer with data to write
 *
 * @param len - length of buf
 *
 * @return nothing
 */
void zb_mac_uart_write(zb_uint8_t* buf, zb_uint_t len)
{
#if defined ZB_HAVE_ASYNC_SERIAL
  zb_osif_serial_send_data(buf, len);
#else
  zb_osif_serial_put_bytes(buf, len);
#endif
}


/**
 * Close com port (file specified in ZB_MAC_TRANSPORT_UART_PATH)
 *
 * @return nothing
 */
void zb_mac_uart_close(void)
{
  zb_osif_serial_deinit();
#ifdef ZB_MAC_TRANSPORT_UART_USE_RST_LINE
  zb_mac_uart_close_rst_line();
#endif  /* ZB_MAC_TRANSPORT_UART_USE_RST_LINE */
}

#endif /*#ifdef ZB_TRANSPORT_LINUX_UART*/
