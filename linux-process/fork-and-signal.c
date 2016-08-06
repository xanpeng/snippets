#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static void child_quit(int signo) {
  wait(NULL);
  puts("child process quit.");
}

void err_sys(const char* fmt, ...) {
  va_list ap;
  fprintf(stderr, fmt, ap);
  exit(1);
}

int main () {
  pid_t pid;
  int i;
  if ((pid = fork()) < 0) {
      err_sys("fork error");
  } else if (pid == 0) { /* child process */
      puts("child process start");
      sleep(3);
      exit (0);
  } else {
      if (signal(SIGCHLD, child_quit) == SIG_ERR)
          err_sys("cannot register signal handler for SIGCHLD");
      puts("signal handler registered for SIGCHLD.");
      /*
      int ret = sleep(30);
      if (ret)
          printf("ret %d, %s\n", ret, strerror(errno));
      */
      for (i = 0; i < 10; ++i) {
        sleep(1);
        printf("#%d: parent is doing work.\n", i);
      }
      puts("parent finished");
  }
  return 0;
}
