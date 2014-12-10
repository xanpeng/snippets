/*
 * gcc dio_read.c -o dio_read -D_GNU_SOURCE
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BUFSIZE 4096
// #define BUFSIZE 111 // fail!

int main()
{
  int fd;
  int ret;
  unsigned char *buf;

  printf("posix_memalign: %d\n", posix_memalign((void**)&buf, 4096, BUFSIZE));
  // buf = malloc(BUFSIZE);   // fail!
  memset(buf, 'c', BUFSIZE);

  fd = open("./dio.data", O_RDONLY | O_DIRECT, 0755);
  if (fd < 0) {
    perror("open dio.data");
    free(buf);
    exit(1);
  }

  do {
    ret = read(fd, buf, BUFSIZE);
    if (ret < 0)
      perror("read dio.data");
  } while (ret > 0);

  free(buf);
  close(fd);

  return 0;
}
