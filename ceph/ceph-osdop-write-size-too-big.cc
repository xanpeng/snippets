// 
// Verify max write size per MOSDOp. Max write size of 1 MOSDOp to ceph osd is defined by osd_max_write_size, default to 90 << 20 = 90MB.
// 
// g++ -g upload-bigfile.cc -o upload-bigfile -lrados
//
// # ls -lh bigfile
// -rw-r--r-- 1 root root 131M Oct 21 09:21 bigfile
// # ./upload-bigfile bigfile
// created a cluster handle.
// read conf done.
// connected cluster.
// created an ioctx.
// buffer size: 136973867
// write object failed: -90 --> #define EMSGSIZE 90 /* Message too long */, return from osd server.
//
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
  int ret = 0;
 
  // upload-bigfile filename
  assert(argc == 2);
  string bigfile = argv[1];
  
  librados::Rados cluster;
  char cluster_name[] = "ceph";
  char user_name[] = "client.admin";
  uint64_t flags;
  ret = cluster.init2(user_name, cluster_name, flags);
  if (ret < 0) {
    cerr << "cluster initialization failed: " << ret << endl;
    return EXIT_FAILURE;
  }
  else {
    cout << "created a cluster handle.\n";
  }
 
  ret = cluster.conf_read_file("/etc/ceph/ceph.conf");
  if (ret < 0) {
    cerr << "read conf failed: " << ret << endl;
    return EXIT_FAILURE;
  }
  else {
    cout << "read conf done.\n";
  }
 
  ret = cluster.connect();
  if (ret < 0) {
    cerr << "connect cluster failed: " << ret << endl;
    return EXIT_FAILURE;
  }
  else {
    cout << "connected cluster.\n";
  }
 
  librados::IoCtx io_ctx;
  const char *pool_name = "imgpool";
  ret = cluster.ioctx_create(pool_name, io_ctx);
  if (ret < 0) {
    cerr << "create ioctx failed: " << ret << endl;
    return EXIT_FAILURE;
  }
  else {
    cout << "created an ioctx.\n";
  }
 
  int fd = open(bigfile.c_str(), O_RDONLY);
  if (fd < 0) {
    cerr << "open bigfile failed\n";
    return EXIT_FAILURE;
  }
  char *buf = new char[1 << 22];
  long buf_size = 0;
  librados::bufferlist bl;
  while (true) {
    int count = read(fd, buf, 1 << 22);
    if (count < 0) {
      cerr << "read bigfile failed.\n";
      return EXIT_FAILURE;
    }
    else if (count == 0) { // eof
      break;
    }
    buf_size += count;
    bl.append(buf, count);
  }
 
  cout << "buffer size: " << buf_size << endl;
  ret = io_ctx.write_full(bigfile, bl);
  if (ret < 0) {
    cerr << "write object failed: " << ret << endl;
    return EXIT_FAILURE;
  }
  else {
    cout << "write object succeed.\n";
  }
 
  return 0;
}
