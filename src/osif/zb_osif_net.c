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
/* PURPOSE: IPS based on Unix domain sockets and poll()
*/

#include "zb_config.h"

//#include "zb_platform.h"

#ifdef UNIX

#define ZB_NSNG
#define ZB_TRACE_FILE_ID 30011
#include "zboss_api_core.h"
#include "zb_osif_unix.h"

#include <signal.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <glob.h>
#include <wchar.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#ifndef MACOSX
#include <malloc.h>
#endif

void osif_net_init()
{
  static int inited = 0;
  struct sigaction act;

  if (!inited)
  {
    setlocale(LC_ALL, "");

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &act, NULL)<0)
    {
      exit(2);
    }
  }
}

char *osif_getenv(char *key, char *def)
{
  char *val = NULL;

  if (key)
  {
    val = getenv(key);
  }
  if (!val)
  {
    val = def;
  }
  return val;
}


unsigned osif_getuid()
{
  return getuid();
}

zb_int32_t osif_gethostid()
{
#ifndef ANDROID
  return gethostid();
#else
  return 0x3F000001; /* FIXME: imeplement it properly for bionic */
#endif
}

zb_int_t osif_ipc_create_h(osif_ipc_handle_t *h_p)
{
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 02000000
#endif
  int sock = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (sock >= 0)
  {
    *h_p = sock;
    sock = 0;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "can not create socket %d", (FMT__D, errno));
    sock = -errno;
  }
  return sock;
}


zb_int_t osif_ipc_srv_init(char *key, osif_ipc_handle_t *h_p)
{
  struct sockaddr_un saddr;
  zb_int_t ret;
  zb_char_t file_path[ZB_MAX_FILE_PATH_SIZE];
  osif_ipc_handle_t sock = -1;

  ZB_BZERO(file_path, sizeof(file_path));
  ret = osif_ipc_create_h(&sock);
  if (ret == 0)
  {
    ZB_MEMSET(&saddr, 0, sizeof(struct sockaddr_un));

    saddr.sun_family = AF_UNIX;
    ZB_FILE_PATH_GET_WITH_POSTFIX(ZB_FILE_PATH_BASE_RAMFS_UNIX_SOCKET,
				  UNIX_DOMAIN_SOCKETS_DIR,"%s", file_path);
    snprintf(saddr.sun_path, sizeof(saddr.sun_path), file_path, key);

    ret = bind(sock, (struct sockaddr*)&saddr, sizeof(saddr.sun_family) + strlen(saddr.sun_path));
  }
  if (ret < 0 && errno == EADDRINUSE)
  {
    /* Remove stale socket for this path */
    /* FIXME: TODO: check that somebody is not keeping this socket (use PID file ?) */
    unlink(saddr.sun_path);
    ret = bind(sock, (struct sockaddr*)&saddr, sizeof(saddr.sun_family) + strlen(saddr.sun_path));
  }
  if (ret == 0)
  {
    ret = listen(sock, 8);
  }
  if (ret == 0)
  {
    *h_p = sock;
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "socket error %d", (FMT__D, errno));
    if (sock >= 0)
    {
      close(sock);
      ret = -errno;
    }
  }
  return ret;
}


zb_int_t osif_ipc_accept(osif_ipc_handle_t srv_h, osif_ipc_handle_t *h_p)
{
  struct sockaddr_un saddr;
  zb_int_t h;
  socklen_t saddrlen = sizeof(saddr);

  do
  {
    errno = 0;
    h = accept(srv_h, (struct sockaddr*)&saddr, &saddrlen);
  } while (h < 0 && errno == EAGAIN);
  if (h >= 0)
  {
    *h_p = h;
    h = 0;
  }
  else
  {
    h = -errno;
  }
  return h;
}


zb_int_t osif_ipc_connect(char *key, osif_ipc_handle_t h)
{
  struct sockaddr_un saddr;
  zb_int_t ret;
  zb_char_t file_path[ZB_MAX_FILE_PATH_SIZE];

  ZB_BZERO(file_path, sizeof(file_path));
  ZB_MEMSET(&saddr, 0, sizeof(struct sockaddr_un));
  saddr.sun_family = AF_UNIX;
  ZB_FILE_PATH_GET_WITH_POSTFIX(ZB_FILE_PATH_BASE_RAMFS_UNIX_SOCKET,
				UNIX_DOMAIN_SOCKETS_DIR,"%s", file_path);
  snprintf(saddr.sun_path, sizeof(saddr.sun_path), file_path, key);

  do
  {
    errno = 0;
    ret = connect(h, (struct sockaddr*)&saddr, sizeof(saddr.sun_family) + strlen(saddr.sun_path));
  }
  while (ret < 0 && errno == EAGAIN);
  if (ret < 0)
  {
    ret = -errno;
  }

  return ret;
}


zb_int_t osif_tcp_create_h(osif_ipc_handle_t *h_p)
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock >= 0)
  {
    *h_p = sock;
    sock = 0;
  }
  else
  {
    sock = -errno;
  }
  return sock;
}


zb_int_t osif_tcp_connect(char *host, unsigned port, osif_ipc_handle_t h)
{
  zb_int_t ret = 0;
  struct addrinfo hint;
  struct addrinfo *ai;

  /* resolve host name */
  ZB_BZERO(&hint, sizeof(hint));
  /* wants ipv4 tcp only */
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  ret = getaddrinfo(host, NULL/*don't search for a port*/, &hint, &ai);
  if (ret == 0)
  {
    ((struct sockaddr_in *)((void*)ai->ai_addr))->sin_port = htons(port);
    do
    {
      errno = 0;
      ret = connect(h, ai->ai_addr, ai->ai_addrlen);
    }
    while (ret < 0 && errno == EAGAIN);
    if (ret < 0)
    {
      ret = -errno;
    }
    freeaddrinfo(ai);
  }

  return ret;
}

zb_int_t osif_tcp_connect_w_timeout(char *host, unsigned port, osif_ipc_handle_t h, zb_uint8_t timeout_sec)
{
  zb_int_t ret = 0;
  struct addrinfo hint;
  struct addrinfo *ai;

  /* resolve host name */
  ZB_BZERO(&hint, sizeof(hint));
  /* wants ipv4 tcp only */
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  ret = getaddrinfo(host, NULL/*don't search for a port*/, &hint, &ai);

  if (ret == 0)
  {
    zb_int_t flags;

    ((struct sockaddr_in *)((void*)ai->ai_addr))->sin_port = htons(port);
    flags = fcntl(h, F_GETFL, 0);
    fcntl(h, F_SETFL, flags | O_NONBLOCK);

    do
    {
      if ((ret = connect(h, ai->ai_addr, ai->ai_addrlen)) < 0)
      {
        if (errno != EINPROGRESS)
        {
          break;
        }
      }

      if (ret == 0)
      {
        break;
      }

      {
        struct timeval tv;
        fd_set wfd, efd;

        FD_ZERO(&wfd);
        FD_SET(h, &wfd);

        FD_ZERO(&efd);
        FD_SET(h, &efd);

        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;

        if ((ret = select(h+1, NULL, &wfd, &efd, &tv)) < 0)
        {
          // select failed
          break;
        }

        if (ret == 0)
        {
          // timeout
          errno = ETIMEDOUT;
          ret = RET_ERROR;
          break;
        }

        if (FD_ISSET(h, &efd))  // connect failed
        {
          zb_int_t  so_error;
          zb_uint_t len;

          len = sizeof(int);
          getsockopt(h, SOL_SOCKET, SO_ERROR, &so_error, &len);
          errno = so_error;
          ret = RET_ERROR;
          break;
        }
        // connected !
        ret = RET_OK;
      }
    }
    while (0);

    if (ret < 0)
    {
      ret = -errno;
    }
    freeaddrinfo(ai);

    // restore file status flags
    fcntl(h, F_SETFL, flags);
  }

  return ret;
}

zb_int_t osif_ipc_write(osif_ipc_handle_t h, const void *buf, zb_int_t len)
{
  zb_int_t ret;

  do
  {
    errno = 0;
    ret = write(h, buf, len);
  }
  while (ret < 0 && errno == EAGAIN);
  if (ret < 0)
  {
    ret = -errno;
  }

  return ret;
}


zb_int_t osif_ipc_read(osif_ipc_handle_t h, void *buf, zb_int_t len)
{
  zb_int_t ret;

  do
  {
    errno = 0;
    ret = read(h, buf, len);
  }
  while (ret < 0 && errno == EAGAIN);
  if (ret < 0)
  {
    ret = -errno;
  }

  return ret;
}


void osif_ipc_close(osif_ipc_handle_t h)
{
  close(h);
}

zb_int_t osif_ipc_wait_for_io(osif_wait_control_t wait_control, zb_int_t n_handlers, zb_uint_t timeout_us)
{
  /* Use select() fror delays in NSNG: it have better precision */
#ifndef ZB_NSNG
  zb_uint_t timeout_ms;
  int ret;
  struct pollfd *pfd = (struct pollfd *)wait_control;

  if (timeout_us == (zb_uint_t)~0)
  {
    timeout_ms = -1;
  }
  else
  {
//    timeout_ms = (timeout_us + 499) / 1000;
    /* Do not round, else got too big timeouts in nsng. Better have zero timeout */
    timeout_ms = timeout_us / 1000LL;
  }
  errno = 0;
  ret = poll(pfd, n_handlers, timeout_ms);
  if (ret < 0)
  {
    ret = -errno;
  }
  return ret;
#else
  /* Use select() instead of poll() because it has us accuracy delay */
  
  struct pollfd *pfd = (struct pollfd *)wait_control;
  fd_set readfds;
  fd_set writefds;
  struct timeval tv;
  struct timeval *tvp = NULL;
  int i;
  int ret = 0;
  int max_fd = 0;

  if (timeout_us != (zb_uint_t)~0)
  {
    tv.tv_sec = timeout_us / 1000000ll;
    tv.tv_usec = timeout_us % 1000000ll;
    tvp = &tv;
  }
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);

  for (i = 0 ; i < n_handlers ; ++i)
  {
    if (pfd[i].fd > max_fd)
    {
      max_fd = pfd[i].fd;
    }
    if (pfd[i].events & POLLIN)
    {
      FD_SET(pfd[i].fd, &readfds);
    }
    if (pfd[i].events & POLLOUT)
    {
      FD_SET(pfd[i].fd, &writefds);
    }
  }

    
  /* select has better timer accuracy */
  ret = select(max_fd + 1, &readfds, &writefds, NULL, tvp);

  for (i = 0 ; i < n_handlers ; ++i)
  {
    pfd[i].revents = 0;
    if (FD_ISSET(pfd[i].fd, &readfds))
    {
      pfd[i].revents = POLLIN;
    }
    if (FD_ISSET(pfd[i].fd, &writefds))
    {
      pfd[i].revents |= POLLOUT;
    }
  }

  return ret;
#endif
}


osif_ipc_handle_t osif_ipc_get_handler(osif_wait_control_t wait_control, zb_int_t i)
{
  struct pollfd *pfd = (struct pollfd *)wait_control;
  return pfd[i].fd;
}


zb_int_t osif_ipc_signaled(osif_wait_control_t wait_control, zb_int_t i, zb_uint_t *mask_p)
{
  struct pollfd *pfd = (struct pollfd *)wait_control;
  zb_uint_t mask = 0;


  if (pfd[i].revents & POLLIN)
  {
    mask |= OSIF_IPC_SIGNAL_RX;
  }
  if (pfd[i].revents & POLLOUT)
  {
    mask |= OSIF_IPC_SIGNAL_TX;
  }
  if (pfd[i].revents & POLLERR)
  {
    mask |= OSIF_IPC_SIGNAL_ERR;
  }
  if (pfd[i].revents & POLLHUP)
  {
    mask |= OSIF_IPC_SIGNAL_HUP;
  }

  if (mask)
  {
    *mask_p = mask;
  }
  return !!mask;
}


zb_ret_t osif_ctrl_pipe_init(osif_control_pipe_t *ctrl)
{
  int ret;
  ret = pipe(*ctrl);
  if (ret == 0)
  {
    /* Put 'write' pipe side into nonblock: we do not want to block due to pipe
     * full because in such case another pipe side must be awaked already. */
    long sfl;
    sfl = fcntl((*ctrl)[1], F_GETFL);
    sfl |= O_NONBLOCK;
    fcntl((*ctrl)[1], F_SETFL, sfl);
    /* Put 'read' pipe side into nonblock too. */
    sfl = fcntl((*ctrl)[0], F_GETFL);
    sfl |= O_NONBLOCK;
    fcntl((*ctrl)[0], F_SETFL, sfl);
  }
  if (ret != 0)
  {
    ret = SYSTEM_ERROR_CODE(errno);
  }
  return ret;
}


zb_ret_t osif_ctrl_pipe_deinit(osif_control_pipe_t ctrl)
{
  close(ctrl[0]);
  close(ctrl[1]);
  return RET_OK;
}


zb_ret_t osif_ctrl_pipe_event_set(osif_control_pipe_t ctrl, zb_char_t e)
{
  int ret = 0;
  do
  {
    ret = !(write(ctrl[1], &e, 1) == 1);
  }
  while (ret
         && errno == EINTR);
  if (ret)
  {
    if (errno == EAGAIN)
    {
      /* if pipe is full, we need not write to it */
      ret = 0;
    }
    else
    {
      ret = SYSTEM_ERROR_CODE(errno);
    }
  }
  return ret;
}

zb_ret_t osif_ctrl_pipe_exit_event_set(osif_control_pipe_t ctrl)
{
  return osif_ctrl_pipe_event_set(ctrl, 'q');
}

void osif_ctrl_pipe_event_read(osif_control_pipe_t ctrl, zb_char_t *e)
{
/* 10/10/2018 EE CR:MAJOR Check read result or at least zero *e before call */
  *e = 0;
  SUPPRESS_UNUSED_RETURN(read(ctrl[0], e, 1));
}

void osif_ctrl_pipe_event_flush(osif_control_pipe_t ctrl)
{
  zb_char_t e;

  osif_ctrl_pipe_event_read(ctrl, &e);
}


osif_ipc_handle_t osif_ctrl_pipe_get_handler(osif_control_pipe_t ctrl)
{
  return ctrl[0];
}


void osif_ipc_init_wait(osif_wait_control_t wait_control, zb_int_t nfds)
{
  struct pollfd *pfd = (struct pollfd *)wait_control;
  int i;

  for (i = 0; i < nfds; i++)
  {
    pfd[i].fd = -1;
    pfd[i].events = 0;
    pfd[i].revents = 0;
  }
}

void osif_ipc_setup_wait(osif_wait_control_t wait_control, zb_int_t i, osif_ipc_handle_t h, zb_uint8_t actions)
{
  struct pollfd *pfd = (struct pollfd *)wait_control;

  pfd[i].fd = h;
  pfd[i].events = 0;
  pfd[i].revents = 0;

  if(actions & OSIF_IPC_SIGNAL_RX)
  {
    pfd[i].events |= POLLIN;
  }
  if(actions & OSIF_IPC_SIGNAL_TX)
  {
    pfd[i].events |= POLLOUT;
  }
}

void osif_ipc_remove_wait(osif_wait_control_t wait_control, zb_int_t i)
{
  struct pollfd *pfd = (struct pollfd *)wait_control;

  pfd[i].fd = -1;
  pfd[i].events = 0;
  pfd[i].revents = 0;
}

/* 03/10/2015 CR [MZ] Block start. */
/* 03/05/2015 CR:Major [EE] No need that call. Need getsockname() instead. */
/* 03/10/2015 CR:MAJOR [MZ] Fixed. */
zb_ret_t osif_net_get_local_ip_address(osif_ipc_handle_t *socket_p, zb_char_t *ip_address_p)
{
  struct sockaddr_in socket_address;
  socklen_t socket_address_length = sizeof(socket_address);

  ZB_BZERO(&socket_address, sizeof(socket_address));
  if (!getsockname(*socket_p, (struct sockaddr *)&socket_address, &socket_address_length))
  {
    /* 03/12/2015 CR:MAJOR [EE] socket_address_length has no relation to the
       length of inet_ntoa result. Use strcpy here. */
    /* 03/13/2015 CR:MAJOR [MZ] Fixed. */
    strcpy(ip_address_p, inet_ntoa(socket_address.sin_addr));
    return RET_OK;
  }
  else
  {
    return RET_ERROR;
  }
}
/* 03/10/2015 CR [MZ] Block end. */

zb_ret_t osif_net_get_mac_address(unsigned char * mac_address, char *ifname)
{
  struct ifreq ifr;
  struct ifconf ifc;
  char buf[1024];
  zb_ret_t ret = RET_ERROR;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock == -1)
  {
    return ret;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
  {
    /* handle error */
  }
  else
  {
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
      strcpy(ifr.ifr_name, it->ifr_name);
      if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
      {
        if(ifname != NULL)
        {
          if(!strcmp(ifr.ifr_name, ifname))
          {
            ret = ioctl(sock, SIOCGIFHWADDR, &ifr) ? RET_ERROR : RET_OK;
            break;
          }
        }
        else if (! (ifr.ifr_flags & IFF_LOOPBACK))
        {
          if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
          {
            ret = RET_OK;
            break;
          }
        }
      }
      else
      {
        /* handle error */
      }
    }
  }
  if (ret == RET_OK)
  {
    memcpy(mac_address, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_SIZE);
  }
  return ret;
}

zb_ret_t osif_net_get_mac_address_sys(unsigned char * mac_address, char *ifname)
{
  FILE *addr_file = NULL;
  zb_ret_t ret = RET_ERROR;
  char fname[128];

  snprintf(fname, sizeof(fname), "/sys/class/net/%s/address", ifname);
  addr_file = fopen(fname, "r");
  if (addr_file)
  {
    SUPPRESS_UNUSED_RETURN(fgets((char*)mac_address, MAC_ADDRESS_WITH_COLONS_STRING_SIZE, addr_file));
    fclose(addr_file);
    ret = RET_OK;
  }
  return ret;
}

zb_ret_t osif_net_get_ip4_address(struct in_addr *ip4_address, char *ifname)
{
  struct ifreq ifr;
  struct ifconf ifc;
  char buf[1024];
  zb_ret_t ret = RET_ERROR;

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock == -1)
  {
    TRACE_MSG(TRACE_OSIF1, "Socket init error", (FMT__0));
    return ret;
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1)
  {
    TRACE_MSG(TRACE_OSIF1, "Socket SIOCGIFCONF error", (FMT__0));
  }
  else
  {
    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
      strcpy(ifr.ifr_name, it->ifr_name);
      if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0)
      {
        if(!strcmp(ifr.ifr_name, ifname))
        {
          ret = ioctl(sock, SIOCGIFADDR, &ifr) ? RET_ERROR : RET_OK;
          break;
        }
        else
        {
          TRACE_MSG(TRACE_OSIF1, "Socket SIOCGIFADDR error", (FMT__0));
        }
      }
      else
      {
        TRACE_MSG(TRACE_OSIF1, "Socket SIOCGIFFLAGS error", (FMT__0));
      }
    }
  }
  if (ret == RET_OK)
  {
    void *addr = &ifr.ifr_addr; /* KLUDGE: allignment warning workaround */
    memcpy(ip4_address, &((struct sockaddr_in*)addr)->sin_addr.s_addr, sizeof(*ip4_address));
  }
  return ret;
}

zb_ret_t osif_net_get_ifname_for_socket(osif_ipc_handle_t h, zb_char_t *buf, zb_uint16_t bufsize)
{
  char      optval[IFNAMSIZ] = {0};
  socklen_t optlen = IFNAMSIZ;

  if (getsockopt(h, SOL_SOCKET, SO_BINDTODEVICE, &optval, &optlen) < 0)
  {
    TRACE_MSG(TRACE_ERROR, "getsockopt error", (FMT__0));
    return RET_NOT_FOUND;
  }
  TRACE_MSG(TRACE_ERROR, "ifname is: %s", (FMT__P, optval));
  ZB_BZERO(buf, bufsize);
  strncpy(buf, optval, bufsize);
  return RET_OK;
#if 0
  struct sockaddr_in addr;
  struct ifaddrs* ifaddr;
  struct ifaddrs* ifa;
  socklen_t addr_len;
  zb_ret_t ret = RET_NOT_FOUND;

  addr_len = sizeof(addr);

  if (getsockname(h, (struct sockaddr*)&addr, &addr_len))
  {
    TRACE_MSG(TRACE_ERROR, "getsockname failed", (FMT__0));
    return RET_ERROR;
  }
  if (getifaddrs(&ifaddr))
  {
    TRACE_MSG(TRACE_ERROR, "getifaddrs failed", (FMT__0));
    return RET_ERROR;
  }

// look which interface contains the wanted IP.
// When found, ifa->ifa_name contains the name of the interface (eth0, eth1, ppp0...)
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr)
    {
      if (AF_INET == ifa->ifa_addr->sa_family)
      {
        struct sockaddr_in* inaddr = (struct sockaddr_in *)ifa->ifa_addr;

        if (inaddr->sin_addr.s_addr == addr.sin_addr.s_addr)
        {
          if (ifa->ifa_name)
          {
            ZB_BZERO(buf, bufsize);
            strncpy(buf, ifa->ifa_name, bufsize);
            ret = RET_OK;
            break;
          }
        }
      }
    }
  }
  freeifaddrs(ifaddr);
  return ret;
#endif
}

#endif /* UNIX */
