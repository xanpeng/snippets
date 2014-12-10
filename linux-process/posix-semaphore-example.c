// Example code for POSIX-semaphore (another type is System-V-semaphore: semget(), semop()).
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define NITER 1000000

int count = 0;
sem_t sem;

void* thread_add(void *a) {
  int i, tmp;
  for (i = 0; i < NITER; ++i) {
    sem_wait(&sem);
    tmp = count;
    tmp = tmp + 1;
    count = tmp;
    sem_post(&sem);
  }
}

int main(int argc, char* argv[]) {
  pthread_t tid1, tid2;

  sem_init(&sem, 0, 1);

  if (pthread_create(&tid1, NULL, thread_add, NULL)) {
    printf("ERROR creating thread 1\n");
    exit(1);
  }

  if (pthread_create(&tid2, NULL, thread_add, NULL)) {
    printf("ERROR creating thread 2\n");
    exit(1);
  }

  if (pthread_join(tid1, NULL)) {
    printf("ERROR joining thread\n");
    exit(1);
  }

  if (pthread_join(tid2, NULL)) {
    printf("ERROR joining thread\n");
    exit(1);
  }

  if (count < 2 * NITER) {
    printf("BOOM! count is [%d], should be %d\n", count, 2*NITER);
  }
  else {
    printf("OK! count is [%d]\n", count);
  }

  sem_destroy(&sem);
  pthread_exit(NULL);
}
