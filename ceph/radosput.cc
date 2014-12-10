#include <iostream>
#include <string>
#include <cassert>
#include <rados/librados.hpp>
 
#include <sys/types.h> // open()
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // read()
using namespace std;
 
int main(int argc, char **argv) {
  // upload-bigfile filename
  assert(argc == 2);
  string bigfile = argv[1];
  
  librados::Rados cluster;
  char cluster_name[] = "ceph";
  char user_name[] = "client.admin";
  uint64_t flags = 0;
  int ret = cluster.init2(user_name, cluster_name, flags);
  assert(ret == 0);
 
  ret = cluster.conf_read_file("/etc/ceph/ceph.conf");
  assert(ret == 0);
 
  ret = cluster.connect();
  assert(ret == 0);
 
  librados::IoCtx io_ctx;
  const char *pool_name = "rbd";
  ret = cluster.ioctx_create(pool_name, io_ctx);
  assert(ret == 0);
 
  int fd = open(bigfile.c_str(), O_RDONLY);
  assert (fd > 0);

  char *buf = new char[1 << 22];
  long buf_size = 0;
  librados::bufferlist bl;
  while (true) {
    int count = read(fd, buf, 1 << 22);
    assert(count >= 0);
    if (count == 0) break;
    buf_size += count;
    bl.append(buf, count);
  }
 
  cout << "buffer size: " << buf_size << endl;
  ret = io_ctx.write_full(bigfile, bl);
  assert (ret >= 0);
 
  return 0;
}
