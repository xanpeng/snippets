#include <err.h>
#include <fcntl.h>

#include <libelf.h>
#include <stdlib.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int fd;
  Elf *e;
  char *k;
  Elf_Kind ek;

  if (argc != 2)
    errx(EX_USAGE, "usage: %s file-name", argv[0]);

  if (elf_version(EV_CURRENT) == EV_NONE)
    errx(EX_SOFTWARE, "ELF library initialization failed %s",
        elf_errmsg(-1));

  if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
    err(EX_NOINPUT, "open \%s\" failed", argv[1]);

  if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
    errx(EX_SOFTWARE, "elf_begin() failed: %s", elf_errmsg(-1));

  ek = elf_kind(e);

  switch (ek)
  {
  case ELF_K_AR:
    k = "ar(1) archive";
    break;
  case ELF_K_ELF:
    k = "elf object";
    break;
  case ELF_K_NONE:
    k = "data";
    break;
  default:
    k = "unrecognized";
    break;
  }

  (void) printf("%s %s\n", argv[1], k);

  (void) elf_end(e);
  (void) close(fd);

  exit(EX_OK);
}
