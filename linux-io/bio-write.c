/**
 * gcc bio_write.c -o bio_write
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

// can be changed freely
#define BUFSIZE 111

int main()
{
  int fd;
  int ret;
  unsigned char *buf;

  // can be block-aligned or not
  // printf("posix_memalign: %d\n", posix_memalign((void**)&buf, 4096, BUFSIZE));
  buf = malloc(BUFSIZE);
  memset(buf, 'c', BUFSIZE);

  fd = open("./bio.data", O_WRONLY | O_CREAT, 0755);
  if (fd < 0) {
    perror("open bio.data");
    free(buf);
    exit(1);
  }

  do {
    ret = write(fd, buf, BUFSIZE);
    if (ret < 0)
      perror("write bio.data");
  } while (0); // changed to 1, make infinite loop, check `free`, free memory is decreasing!

  free(buf);
  close(fd);

  return 0;
}
