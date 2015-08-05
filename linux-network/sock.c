#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "sock.h"

//////// utils

const char* sock_str_ip(uint32_t ip)
{
  static __thread char buffers[4][16];
  static __thread int32_t i = 0;
  char* buffer = buffers[i++ % 4];
  snprintf(buffer, 16, "%d.%d.%d.%d",
      (ip >> 24) & 255,
      (ip >> 16) & 255,
      (ip >> 8) & 255,
      (ip) & 255);
  return buffer;
}

const char* sock_str_addr(struct sockaddr_in* addr)
{
  static __thread char buffers[4][32];
  static __thread int32_t i = 0;
  char *buffer = NULL;
  if (NULL != addr)
  {
    uint32_t ip = ntohl(addr->sin_addr.s_addr);
    int32_t port = ntohs(addr->sin_port);
    buffer = buffers[i++ % 4];
    snprintf(buffer, 32, "%d.%d.%d.%d:%d",
        (ip >> 24) & 255,
        (ip >> 16) & 255,
        (ip >> 8) & 255,
        (ip) & 255,
        port);
  }
  return buffer;
}

const char* psock_str_addr(sockpair_t* sp)
{
  return (NULL == sp) ? NULL : sock_str_addr(&(sp->addr));
}

const char* psock_str_peer(sockpair_t* sp)
{
  return (NULL == sp) ? NULL : sock_str_addr(&(sp->peer));
}

int sock_get_port(struct sockaddr_in* addr)
{
  return (NULL == addr) ? -1 : ntohs(addr->sin_port);
}

int sock_get_ip(struct sockaddr_in* addr)
{
  return (NULL == addr) ? 0 : ntohl(addr->sin_addr.s_addr);
}

//////// raw socket ////////

int sock_open(int domain, int type, int protocol)
{
  return socket(domain, type, protocol);
}

int sock_bind(int sockfd, sockaddr_t *addr)
{
  return bind(sockfd, &addr->addr.sa, addr->addrlen);
}

int socket_listen(int sockfd, int backlog)
{
  return listen(sockfd, backlog);
}

int socket_connect(int sockfd, sockaddr_t *addr)
{
  return connect(sockfd, &addr->addr.sa, addr->addrlen);
}

int socket_accept(int sockfd, sockaddr_t *addr)
{
  addr->addrlen = sizeof(addr->addr);
  return accept(sockfd, &addr->addr.sa, &addr->addrlen);
}

int sock_set_nonblock(int sockfd)
{
  return fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int sock_set_reuse_addr(int sockfd)
{
  int reuseaddr = 1;
  return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
}

int sock_set_reuse_port(int sockfd)
{
#ifdef SO_REUSEPORT
  // The SO_REUSEPORT socket option: https://lwn.net/Articles/542629/
  int reuseport = 1;
  return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));
#else
# warning sock_set_reuseport require SO_REUSEPORT
#endif
  return -1;
}

int socket_set_tcp_nodelay(int sockfd)
{
  // used to disable nagle's algorithm to improve tcp/ip networks and 
  // decrease the number of packets by waiting until an acknowledgment 
  // of previous sent data is received to send the accumulated packets.
  int tcp_nodelay = 1;
  return setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay, sizeof(tcp_nodelay));
}

int socket_set_tcp_nopush(int sockfd)
{
  int tcp_nopush = 1;
#ifdef TCP_NOPUSH
  return setsockopt(sockfd, SOL_SOCKET, TCP_NOPUSH, &tcp_nopush, sizeof(tcp_nopush));
#elif defined(TCP_CORK)
  // If set, don't send out partial frames
  return setsockopt(sockfd, SOL_SOCKET, TCP_CORK, &tcp_nopush, sizeof(tcp_nopush));
#else
# warning sock_set_tcp_nopush require TCP_NOPUSH or TCP_CORK
#endif
}

//////// psock ////////

static int is_ip_addr_(const char* addr)
{
  return -1;
}

static sockpair_t* psock_malloc_()
{
  sockpair_t* sp = (sockpair_t*) malloc(sizeof(sockpair_t));
  if (NULL != sp)
  {
    memset(sp, 0, sizeof(*sp));
    sp->fd = -1;
  }
  return sp;
}

static void psock_free_(sockpair_t* sp)
{
  if (NULL != sp)
  {
    free(sp);
    sp = NULL;
  }
}

sockpair_t* psock_init(int type)
{
  sockpair_t* sp = psock_malloc_();

  if (NULL != sp && -1 == (sp->fd = socket(AF_INET, type, 0)))
    psock_free_(sp);

  return sp;
}

void psock_destroy(sockpair_t* sp)
{
  if (NULL == sp)
    return;
  psock_close(sp);
  psock_free_(sp);
}

int psock_get_fd(sockpair_t* sp)
{
  return (NULL != sp) ? sp->fd : -1;
}

struct sockaddr_in* psock_get_addr(sockpair_t* sp)
{
  return (NULL != sp) ? &(sp->addr) : NULL;
}

struct sockaddr_in* psock_get_peer(sockpair_t* sp)
{
  return (NULL != sp) ? &(sp->peer) : NULL;
}

int psock_set_addr(sockpair_t* sp, const char* addr, int port)
{
  if (NULL == sp)
    return -1;

  int ret = 0;
  memset(&(sp->addr), 0, sizeof(sp->addr));
  sp->addr.sin_family = AF_INET;
  sp->addr.sin_port = htons(port);
  if (NULL == addr || '\0' == addr[0])
    sp->addr.sin_addr.s_addr = htons(INADDR_ANY);
  else if (is_ip_addr_(addr))
    sp->addr.sin_addr.s_addr = inet_addr(addr);
  else
  {
    const uint32_t BUFF_SIZE = 4096;
    char buff[4096];
    struct hostent hostinfo;
    struct hostent *phost = NULL;
    if (0 != gethostbyname_r(addr, &hostinfo, buff, BUFF_SIZE, &phost, &ret))
      ret = -1;
    else
      memcpy(&(sp->addr.sin_addr), *(hostinfo.h_addr_list), sizeof(struct in_addr));
  }

  return ret;
}

int psock_set_peer(sockpair_t* sp, const char* addr, int port)
{
  if (NULL == sp)
    return -1;

  int ret = 0;
  memset(&(sp->peer), 0, sizeof(sp->peer));
  sp->peer.sin_family = AF_INET;
  sp->peer.sin_port = htons(port);
  if (NULL == addr || '\0' == addr[0])
    sp->peer.sin_addr.s_addr = htons(INADDR_ANY);
  else if (is_ip_addr_(addr))
    sp->peer.sin_addr.s_addr = inet_addr(addr);
  else
  {
    const uint32_t BUFF_SIZE = 4096;
    char buff[4096];
    struct hostent hostinfo;
    struct hostent *phost = NULL;
    if (0 != gethostbyname_r(addr, &hostinfo, buff, BUFF_SIZE, &phost, &ret))
      ret = -1;
    else
      memcpy(&(sp->peer.sin_addr), *(hostinfo.h_addr_list), sizeof(struct in_addr));
  }

  return ret;
}

int psock_connect(sockpair_t* sp)
{
  if (NULL == sp)
    return -1;

  if (0 != connect(sp->fd, (struct sockaddr*) &(sp->addr), sizeof(sp->addr)))
    return -1;

  int len = sizeof(struct sockaddr);
  if (0 != getsockname(sp->fd, (struct sockaddr*) &(sp->peer), (socklen_t*) &len))
    return -1;

  return 0;
}

int psock_bind_addr(sockpair_t* sp)
{
  if (NULL == sp)
    return -1;

  if ((0 != sp->addr.sin_port || 0 != sp->addr.sin_addr.s_addr)
      && 0 != bind(sp->fd, (struct sockaddr*) &sp->addr, sizeof(sp->addr)))
    return -1;

  return 0;
}

int psock_bind_peer(sockpair_t* sp)
{
  if (NULL == sp)
    return -1;

  if ((0 != sp->peer.sin_port || 0 != sp->peer.sin_addr.s_addr)
      && 0 != bind(sp->fd, (struct sockaddr*) &sp->peer, sizeof(sp->peer)))
    return -1;

  return 0;
}

int psock_listen(sockpair_t* sp, int backlog)
{
  if (NULL == sp)
    return -1;

  if (0 != listen(sp->fd, backlog))
    return -1;

  return 0;
}

sockpair_t* psock_accept(sockpair_t* sp)
{
  if (NULL == sp)
    return NULL;

  sockpair_t* ret = psock_malloc_();
  if (NULL == ret)
    return NULL;

  int len = sizeof(struct sockaddr);
  if (-1 == (ret->fd = accept(sp->fd, (struct sockaddr*) &(ret->peer), (socklen_t*) &len)))
    psock_free_(ret);

  if (0 != getsockname(ret->fd, (struct sockaddr*) &(ret->addr), (socklen_t*) &len))
    psock_destroy(ret);

  return ret;
}

void psock_close(sockpair_t* sp)
{
  if (NULL == sp || -1 != sp->fd)
    return;

  close(sp->fd);
  sp->fd = -1;
}

int psock_int_option(sockpair_t* sp, int type, int option, int value)
{
  if (NULL == sp)
    return -1;

  if (0 != setsockopt(sp->fd, type, option, (void*) &value, sizeof(value)))
    return -1;

  return 0;
}

int psock_time_option(sockpair_t* sp, int option, int64_t usec)
{
  if (NULL == sp)
    return -1;

  struct timeval tv;
  tv.tv_sec = (int)(usec / 1000000);
  tv.tv_usec = (int)(usec % 1000000);
  if (0 != setsockopt(sp->fd, SOL_SOCKET, option, (void*)&tv, sizeof(tv)))
    return -1;

  return 0;
}

int psock_set_keep_alive(sockpair_t* sp, int on)
{
  return psock_int_option(sp, SOL_SOCKET, SO_KEEPALIVE, on);
}

int psock_set_reuse_addr(sockpair_t* sp, int on)
{
  return psock_int_option(sp, SOL_SOCKET, SO_REUSEADDR, on);
}

int psock_set_linger(sockpair_t* sp, int on, int sec)
{
  if (NULL == sp)
    return -1;

  struct linger lt;
  lt.l_onoff = on;
  lt.l_linger = sec;
  if (0 != setsockopt(sp->fd, SOL_SOCKET, SO_LINGER, (void*)&lt, sizeof(lt)))
    return -1;

  return 0;
}

int psock_set_tcp_nodelay(sockpair_t* sp, int on)
{
  return psock_int_option(sp, IPPROTO_TCP, TCP_NODELAY, on);
}

int psock_set_tcp_quickack(sockpair_t* sp, int on)
{
  return psock_int_option(sp, IPPROTO_TCP, TCP_QUICKACK, on);
}

int psock_set_nonblock(sockpair_t* sp, int on)
{
  if (NULL == sp)
    return -1;

  int flag = 0;
  if (0 >= (flag = fcntl(sp->fd, F_GETFL, NULL)))
    return -1;

  if (on)
    flag |= O_NONBLOCK;
  else
    flag &= ~O_NONBLOCK;
  if (0 != fcntl(sp->fd, F_SETFL, flag))
    return -1;

  return 0;
}

int psock_send_buffer(sockpair_t* sp, int size)
{
  return psock_int_option(sp, SOL_SOCKET, SO_SNDBUF, size);
}

int psock_recv_buffer(sockpair_t* sp, int size)
{
  return psock_int_option(sp, SOL_SOCKET, SO_RCVBUF, size);
}

int psock_send_timeo(sockpair_t* sp, int time)
{
  return psock_time_option(sp, SO_SNDTIMEO, time);
}

int psock_recv_timeo(sockpair_t* sp, int time)
{
  return psock_time_option(sp, SO_RCVTIMEO, time);
}

