#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/utsname.h>

#define BLKGETSIZE64 _IOR(0x12,114,size_t)  /* return device size in bytes (u64 *arg) */

int get_device_blocks(const char *file, int blocksize, uint64_t *retblocks)
{
  int fd, rc = 0;
  int valid_blkgetsize64 = 1;
  struct utsname ut;
  unsigned long long size64;

  fd = open64(file, O_RDONLY);
  if (fd < 0)
    return errno;

  if ((uname(&ut) == 0) &&
      ((ut.release[0] == '2') && (ut.release[1] == '.') &&
       (ut.release[2] < '6') && (ut.release[3] == '.')))
    valid_blkgetsize64 = 0;

  if (valid_blkgetsize64 &&
      ioctl(fd, BLKGETSIZE64, &size64) >= 0) {
    if ((sizeof(*retblocks) < sizeof(unsigned long long)) &&
        ((size64 / blocksize) > 0xFFFFFFFF)) {
      rc = EFBIG;
      goto out;
    }
    *retblocks = size64 / blocksize;
    goto out;
  }

out:
  close(fd);
  return rc;
}

/**
 * gcc get-dev-blocks.c -g -Wall -o get-dev-blocks
 * usage: ./get-dev-blocks /dev/sda
 */
int main(int argc, char *argv[])
{
  int err;
  uint64_t retblocks;

  err = get_device_blocks(argv[1], 4096, &retblocks);
  printf("err: %d, %s: %"PRIu64" blocks\n", err, argv[1], retblocks);

  return 0;
}
