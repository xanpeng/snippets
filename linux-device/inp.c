#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/io.h>
#ifdef __GLIBC__
#include <sys/perm.h>
#endif

#define PORT_FILE "/dev/port"

char* progname;

#ifdef __i386__
static int read_and_print_one(unsigned int port, int size) {
  static int iopldone = 0;

  if (port > 1024) {
    if (!iopldone && iopl(3)) {
      fprintf(stderr, "%s: iopl(): %s\n", progname, strerror(errno));
      return 1;
    }
    iopldone++;
  } else if (ioperm(port, size, 1)) {
    fprintf(stderr, "%s: ioperm(%x): %s\n", progname, port, strerror(errno));
    return 1;
  }

  if (size == 4) printf("%04x: %08x\n", port, inl(port));
  else if (size == 2) printf("%04x: %04x\n", port, inw(port));
  else printf("%04x: %02x\n", port, inb(port));
  return 0;
}
#else
static int read_and_print_one(unsigned int port, int size) {
  static int fd = -1;
  unsigned char b;
  unsigned short w;
  unsigned int l;

  if (fd < 0) fd = open(PORT_FILE, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "%s: %s: %s\n", progname, PORT_FILE, strerror(errno));
    return 1;
  }
  lseek(fd, port, SEEK_SET);

  if (size == 4) {
    read(fd, &l, 4);
    printf("%04x: 0x%08x\n", port, l);
  } else if (size == 2) {
    read(fd, &w, 2);
    printf("%04x: 0x%04x\n", port, w & 0xffff);
  } else {
    read(fd, &b, 1);
    printf("%04x: 0x%02x\n", port, b & 0xff);
  }

  return 0;
}
#endif

int main(int argc, char** argv) {
  unsigned int i, n, port, size, error = 0;

  progname = argv[0];
  switch (progname[strlen(progname)-1]) {
  case 'w':
    size = 2; break;
  case 'l':
    size = 4; break;
  case 'b':
  case 'p':
  default:
    size = 1;
  }

  setuid(0);
  for (i = 1; i < argc; ++i) {
    if (sscanf(argv[i], "%x%n", &port, &n) < 1 || n != strlen(argv[i])) {
      fprintf(stderr, "%s: argument \"%s\" is not a hex number\n", argv[0], argv[i]);
      ++error;
      continue;
    }
    if (port & (size-1)) {
      fprintf(stderr, "%s: argument \"%s\" is not properly aligned\n", argv[0], argv[i]);
      ++error;
      continue;
    }
    error += read_and_print_one(port, size);
  }

  exit(error ? 1 : 0);
}
