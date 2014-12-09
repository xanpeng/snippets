#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

static void handler(int sig) {
  printf("PID %ld: caught signal %2d (%s)\n", (long) getpid(), sig, strsignal(sig));
}

int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);       /* Make stdout unbuffered */

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  if (sigaction(SIGHUP, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  pid_t child_pid = fork();
  if (child_pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (child_pid == 0 && argc > 1)
    if (setpgid(0, 0) == -1) {       /* Move to new process group */
      perror("setpgid");
      exit(EXIT_FAILURE);
    }

  printf("PID=%ld; PPID=%ld; PGID=%ld; SID=%ld\n", (long) getpid(), (long) getppid(), (long) getpgrp(), (long) getsid(0));

  alarm(60);                  /* An unhandled SIGALRM ensures this process will die if nothing else terminates it */
  for(;;) {                   /* Wait for signals */
    pause();
    printf("%ld: caught SIGHUP\n", (long) getpid());
  }
}
