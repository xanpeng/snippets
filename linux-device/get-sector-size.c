#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/fd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define BLKSSZGET  _IO(0x12,104)/* get block device sector size */

void get_sectsize(const char *file, uint32_t *sectsize)
{
  int fd = open64(file, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "open64 failed");
    return;
  }

  ioctl(fd, BLKSSZGET, sectsize);
  close(fd);
}

/**
 * gcc get_sectsize.c -Wall -o get_sectsize
 * ./get_sectsize /dev/sdj1
 */
int main(int argc, char *argv[])
{
  uint32_t sectsize;

  get_sectsize(argv[1], &sectsize);
  printf("sectsize: %d\n", sectsize);

  return 0;
}
