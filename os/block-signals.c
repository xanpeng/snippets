/// code based on man 3 pthread_sigmask
/// gcc block-signals.c -pthread
/*
           $ ./a.out &
           [1] 5423
           $ kill -QUIT %1
           Signal handling thread got signal 3
           $ kill -USR1 %1
           Signal handling thread got signal 10
           $ kill -TERM %1
           [1]+  Terminated              ./a.out
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

static void* sig_thread(void *arg) {
  sigset_t *set = arg;
  int s, sig;

  for (;;) {
    s = sigwait(set, &sig); // wait for a signal
    assert(s == 0);
    printf("Signal handling thread got signal %d\n", sig);
  }
}

/// if old_sigset != NULL, previous sigset is stored into it 
static void block_signals(const int *siglist, sigset_t *sigset, sigset_t *old_sigset) {
  if (!siglist)
    sigfillset(sigset); // if siglist==NULL, put all signals in
  else {
    int i = 0;
    sigemptyset(sigset);
    while (siglist[i]) {
      sigaddset(sigset, siglist[i]);
      ++i;
    }
  }

  int ret = pthread_sigmask(SIG_BLOCK, sigset, old_sigset);
  assert(ret == 0);
}

int main() {
  pthread_t thread;
  sigset_t sigset;
  int s;

  // Block SIGQUIT and SIGUSR1; other threads created by main() will inherit a copy of the signal mask.
  int siglist[2] = { SIGQUIT, SIGUSR1 };
  block_signals(siglist, &sigset, NULL); // signals of current thread (main thread) are blocked

  // Child thread's SIGQUIT+SIGUSR1 are blocked inheritly
  s = pthread_create(&thread, NULL, &sig_thread, (void *) &sigset);
  assert(s == 0);

  // Main thread carries on to create other threads and/or do other work

  pause();
}
