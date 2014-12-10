/**
 * code stolen from ocfs2-tools/libocfs2/openfs.c
 *
 * gcc  ocfs2-dump-super.c -o ocfs2-dump-super
 * ./ocfs2-dump-super /dev/sda
 *
 * */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCK_SIZE 4096

static int unix_io_read_block(char *dev, int64_t blkno, int count, char *data)
{
  int ret;
  ssize_t size, total, rd;
  uint64_t location;
  int dev_fd;

  dev_fd = open64(dev, O_RDONLY);
  if (dev_fd < 0)
    return -1;

  size = count * BLOCK_SIZE; // default block size 4096 bytes
  location = blkno * BLOCK_SIZE;
  total = 0;
  while (total < size) {
    /*
Q: pread64和read有什么区别？
A：pread64的p应该指pthread，二者的区别很明显，前者多一个参数offset，后者没有这个参数，默认是从头部开始读取的。

Q: pread64调用一次，发起一次disk io？一次可以读多少？
A：简单地说，pread64/read不用考虑这个问题，因为它们在内核中是调用vfs_read读取的。
我用“python write_file.py -f file-1g -s 1073741824”创建一个文件，写文件的方式不是direct io，
再用“./hugeread file-1g”调用pread64读取，一次就读完1G了。
所以pread64调用多少次，一次读多少，简单的说不是它的问题，它只是一个“傀儡”。

但对于块设备文件，open时没有指定O_DIRECT，难道就会读page cache了？不一定，因为page cache凭什么缓存你设备的raw内容？
--[10-29 update] 还真会，参考ULK3 ch15 “The page cache”. page cache中的raw内容属于设备文件的inode。
<del>所以，我理解对于块设备，如果只有读取操作，open没有必要指定O_DIRECT。</del>
如果有写操作，如果不是O_DIRECT，那么就很可能没有真正写     ======> to be exampled

注1：write_file.py在python-misc库下。
注2：hugeread的代码放到本例最后的注释中。

*/
    rd = pread64(dev_fd, data + total, size - total, location + total);
    if (rd < 0) {
      close(dev_fd);
      return -1;
    }
    if (rd == 0)
      return 0;
    total += rd;
    printf("pread64, size(read/total): %d / %d, new offset: %d\n", total, size, location + total);
  }

  close(dev_fd);
  return 0;
}

static void malloc_blocks(int num_blocks, void **ptr)
{
  int ret;
  size_t bytes = num_blocks * BLOCK_SIZE;

  // int posix_memalign(void **memptr, size_t alignment, size_t size);
  ret = posix_memalign(ptr, BLOCK_SIZE, bytes);

  if (ret)
    abort();
}

int main(int argc, char *argv[])
{
  void *blk;

#define NUM_BLOCKS 1    // superblock takes 1 block of disk space
  malloc_blocks(NUM_BLOCKS, &blk);

  // default start block # 2
  printf("read block: %d\n", unix_io_read_block(argv[1], 2, NUM_BLOCKS, blk));

  return 0;
}

/* huge-read.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
int fd;
#define COUNT 1024*1024*1024
char* buf = malloc(COUNT);
if (!buf) {
fprintf(stderr, "malloc failed\n");
return 1;
}

fd = open(argv[1], O_RDONLY);
if (fd < 0) {
fprintf(stderr, "open failed\n");
return 1;
}

// printf("pread64: %d\n", pread64(fd, buf, COUNT, 0));
printf("read: %d\n", read(fd, buf, COUNT));

return 0;
}
*/
