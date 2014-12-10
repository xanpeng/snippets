// c program to implement a fifo client
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

struct fifo_cmd {
  pid_t pid;
  char cmd[100];
};

int main(int argc, char* argv[]) {
  int fd;
  struct fifo_cmd cmd;

  if((fd = open("/tmp/server", O_WRONLY)) < 0) {
    perror("open fail: ");
    exit(-1);
  }

  cmd.pid = getpid();
  while(1) {
    printf("%%: ");
    fgets(cmd.cmd, sizeof(cmd.cmd), stdin);
    cmd.cmd[strlen(cmd.cmd) - 1] = 0;
    if(write(fd, &cmd, sizeof(cmd)) < 0) {
      perror("write fail: ");
      exit(-1);
    }
  }
}
