// based on http://dirlt.com/libev.html

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <string>
#include <ev.h>

namespace common {

#define D(exp, fmt, ...) do {             \
  if(!(exp)){                             \
    fprintf(stderr, fmt, ##__VA_ARGS__);  \
    abort();                              \
  }                                       \
} while(0)

static void setnonblock(int fd) {
  fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);
}

static void setreuseaddr(int fd) {
  int ok=1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
}

static void setaddress(const char* ip, int port, struct sockaddr_in* addr) {
  bzero(addr, sizeof(*addr));
  addr->sin_family = AF_INET;
  inet_pton(AF_INET, ip, &(addr->sin_addr));
  addr->sin_port = htons(port);
}

static std::string address_to_string(struct sockaddr_in* addr) {
  char ip[128];
  inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
  char port[32];
  snprintf(port, sizeof(port), "%d", ntohs(addr->sin_port));
  std::string r;
  r = r+"("+ip+":"+port+")";
  return r;
}

static int new_tcp_server(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  D(fd>0, "socket failed(%m)\n");
  setnonblock(fd);
  setreuseaddr(fd);
  sockaddr_in addr;
  setaddress("0.0.0.0", port, &addr);
  bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  listen(fd, 64); // backlog = 64
  return fd;
}

static int new_tcp_client(const char* ip, int port){
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  setnonblock(fd);
  sockaddr_in addr;
  setaddress(ip, port, &addr);
  connect(fd, (struct sockaddr*)(&addr), sizeof(addr));
  return fd;
}

}; // namespace common

static void do_accept(struct ev_loop* reactor, ev_io* w, int events){
  struct sockaddr_in addr;
  socklen_t addr_size = sizeof(addr);
  int conn = accept(w->fd, (struct sockaddr*)&addr, &addr_size);
  std::string r = common::address_to_string(&addr);
  fprintf(stderr, "accept %s\n", r.c_str());
  close(conn);
}

int main() {
  struct ev_loop* reactor = ev_loop_new(EVFLAG_AUTO);
  int fd = common::new_tcp_server(34567);
  ev_io w;
  ev_io_init(&w, do_accept, fd, EV_READ);
  ev_io_start(reactor, &w);
  ev_run(reactor, 0);
  close(fd);
  ev_loop_destroy(reactor);
}

