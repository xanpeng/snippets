#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct {
  socklen_t addrlen;
  union {
    struct sockaddr         sa;
    struct sockaddr_in      si;
    struct sockaddr_in6     s6;
    struct sockaddr_storage ss;
  } addr;
} sockaddr_t;

typedef struct {
  int fd;
  struct sockaddr_in addr;
  struct sockaddr_in peer;
} sockpair_t;

///////// utils

extern const char* sock_str_ip(uint32_t ip);
extern const char* sock_str_addr(struct sockaddr_in* addr);
extern int sock_get_port(struct sockaddr_in* addr);
extern int sock_get_ip(struct sockaddr_in* addr);

extern const char* psock_str_addr(sockpair_t* sp);
extern const char* psock_str_peer(sockpair_t* sp);

//////// raw socket

extern int sock_open(int domain, int type, int protocol);
extern int sock_bind(int sockfd, sockaddr_t *addr);
extern int sock_listen(int sockfd, int backlog);
extern int sock_connect(int sockfd, sockaddr_t *addr);
extern int sock_accept(int sockfd, sockaddr_t *addr);

extern int sock_set_nonblock(int sockfd);
extern int sock_set_reuse_addr(int sockfd);
extern int sock_set_reuse_port(int sockfd);
extern int sock_set_tcp_nodelay(int sockfd);
extern int sock_set_tcp_nopush(int sockfd);

//////// psock: paired socket

extern sockpair_t* psock_init(int type);
extern void psock_destroy(sockpair_t* sp);
extern int psock_get_fd(sockpair_t* sp);
extern struct sockaddr_in* psock_get_addr(sockpair_t* sp);
extern struct sockaddr_in* psock_get_peer(sockpair_t* sp);
extern int psock_set_addr(sockpair_t* sp, const char* addr, int port);
extern int psock_set_peer(sockpair_t* sp, const char* addr, int port);

extern int psock_connect(sockpair_t* sp);
extern int psock_bind_addr(sockpair_t* sp);
extern int psock_bind_peer(sockpair_t* sp);
extern int psock_listen(sockpair_t* sp, int backlog);
extern sockpair_t* psock_accept(sockpair_t* sp);
extern void psock_close(sockpair_t* sp);

extern int psock_int_option(sockpair_t* sp, int type, int option, int value);
extern int psock_time_option(sockpair_t* sp, int option, int64_t usec);

extern int psock_set_keep_alive(sockpair_t* sp, int on);
extern int psock_set_reuse_addr(sockpair_t* sp, int on);
extern int psock_set_linger(sockpair_t* sp, int on, int sec);
extern int psock_set_tcp_nodelay(sockpair_t* sp, int on);
extern int psock_set_tcp_quickack(sockpair_t* sp, int on);
extern int psock_set_nonblock(sockpair_t* sp, int on);
extern int psock_set_send_buffer(sockpair_t* sp, int size);
extern int psock_set_recv_buffer(sockpair_t* sp, int size);
extern int psock_set_send_timeo(sockpair_t* sp, int time);
extern int psock_set_recv_timeo(sockpair_t* sp, int time);
