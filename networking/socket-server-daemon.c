// ref https://github.com/zhicheng/libloop

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define die(fmt, ...) do {  \
  fprintf(stderr, "%s:%d "fmt"\n", __FILE__, __LINE__, ##__VAR_ARGS__); \
  exit(0);                  \
} while (0)

#define die_unless(a) do {  \
  if (!(a)) {               \
    die("%s", #a);          \
  }                         \
} while (0)

typedef struct {
  socklen_t addrlen;
  union {
    struct sockaddr         sa;
    struct sockaddr_in      si;
    struct sockaddr_in6     s6;
    struct sockaddr_storage ss;
  } addr;
} socket_addr_t;

int socket_open(int domain, int type, int protocl) {
  return socket(domain, type, protocl);
}

int socket_set_nonblock(int sockfd) {
  return fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int socket_set_reuseaddr(int sockfd) {
  int reuseaddr;
  reuseaddr = 1;
  return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
}

int socket_set_reuseport(int sockfd) {
#ifdef SO_REUSEPORT
  int reuseport;
  reuseport = 1;
  return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));
#else
# warning socket_set_reuseport require SO_REUSEPORT
#endif /* SO_REUSEPORT */
  return -1;
}

int socket_set_tcp_nodelay(int sockfd) {
  int tcp_nodelay;
  tcp_nodelay = 1;
  return setsockopt(sockfd, SOL_SOCKET, TCP_NODELAY, &tcp_nodelay, sizeof(tcp_nodelay));
}

int socket_set_tcp_nopush(int sockfd) {
  int tcp_nopush;
  tcp_nopush = 1;
#ifdef TCP_NOPUSH
  return setsockopt(sockfd, SOL_SOCKET, TCP_NOPUSH, &tcp_nopush, sizeof(tcp_nopush));
#elif defined(TCP_CORK)
  return setsockopt(sockfd, SOL_SOCKET, TCP_CORK, &tcp_nopush, sizeof(tcp_nopush));
#else
# warning socket_set_tcp_nopush require TCP_NOPUSH or TCP_CORK
#endif
}

int socket_bind(int sockfd, socket_addr_t *addr) {
  return bind(sockfd, &addr->addr.sa, addr->addrlen);
}

int socket_listen(int sockfd, int backlog) {
  return listen(sockfd, backlog);
}

int socket_connect(int sockfd, socket_addr_t *addr) {
        return connect(sockfd, &addr->addr.sa, addr->addrlen);
}

int socket_accept(int sockfd, socket_addr_t *addr) {
  addr->addrlen = sizeof(addr->addr);
  return accept(sockfd, &addr->addr.sa, &addr->addrlen);
}

int main(void) {
  int fd, err;
  socket_addr_t addr;

  signal(SIGPIPE, SIG_IGN);

  fd = socket_open(AF_INET6, SOCK_STREAM, 0);
  die_unless(fd != -1);

  err = socket_set_nonblock(fd);
  die_unless(err == 0);

  err = socket_set_reuseaddr(fd);
  die_unless(err == 0);

  err = socket_set_reuseport(fd);
  die_unless(err == 0);

  err = socket_set_tcp_nodelay(fd);
  die_unless(err == 0);

  err = socket_set_tcp_nopush(fd);
  die_unless(err == 0);

  err = socket_addr(&addr, "::", 8080);
  die_unless(err == 0);

  err = socket_bind(fd, &addr);
  die_unless(err == 0);

  err = socket_listen(fd, 32);
  die_unless(err == 0);

  // loop
  
  return 0;
}
