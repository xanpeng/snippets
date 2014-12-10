// gcc dio_write.c -o dio_write -D_GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
 
void do_dio(char *path, int is_blk) {
  // buffer requirement for dio:
  // 1. start addr = n * logical_block_size
  // 2. buffer size = n * logical_block_size
  //
  // logical_block_size is the unit used by the kernel for rw.
  // - /sys/block/vda/queue/logical_block_size 512
  // physical_block_size is the unit which disk controllers use for rw.
  // - /sys/block/vda/queue/physical_block_size 512
  unsigned char *buf;
  const int bufsize = 1024;
  printf("posix_memalign: %d\n", posix_memalign((void**)&buf, 512, bufsize));
 
  // malloc cannot fit buffer requirement
  // buf = malloc(bufsize);  // fail!
 
  memset(buf, 'c', bufsize);
 
  int fd = -1;
  if (is_blk) fd = open(path, O_RDWR | O_DIRECT | O_DSYNC, 0644);
  else fd = open(path, O_RDWR | O_DIRECT | O_DSYNC | O_CREAT, 0755);
  if (fd < 0) {
    perror("open dio.data");
    free(buf);
    exit(1);
  }
 
  if (write(fd, buf, bufsize) < 0)
    perror("write dio.data");
 
  free(buf);
  close(fd);
}
 
int main() {
  printf("dio on file\n");
  do_dio("dio.data", 0);
  printf("\ndio on block device\n");
  do_dio("/dev/vde4", 1);
  return 0;
}
