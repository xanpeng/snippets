// c program for testing pipe operations
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(void) {
  int n;
  int fd[2];
  pid_t pid;
  char line[1024];

  if(pipe(fd) < 0) {
    fprintf(stderr, "pipe error: %s\n", strerror(errno));
    exit(-1);
  }

  if((pid = fork()) < 0) {
    fprintf(stderr, "fork error: %s\n", strerror(errno));
    exit(-1);
  }
  else if(pid > 0) { /* Parent */
    close(fd[1]);
    n = read(fd[0], line, 1024);
    write(STDOUT_FILENO, line, n);
  }
  else { /* Child */
    close(fd[0]);
    write(fd[1], "Hello World\n", 12);
  }

  return 0;
}
