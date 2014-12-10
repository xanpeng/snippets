// c program for testing mutex, mutex can be used without condition.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUMTHREADS 4
#define VECLEN 100

typedef struct {
  double *a;
  double *b;
  double sum;
  int veclen;
} DOTDATA;
DOTDATA dotstr;
pthread_t call_thread[NUMTHREADS];
pthread_mutex_t mutexsum;

void *dotprod(void *arg) {
  int i, start, end, len, offset;
  double mysum, *x, *y;

  offset = (long)arg;
  len = dotstr.veclen;
  start = offset * len; end = start + len;
  x = dotstr.a; y = dotstr.b;

  mysum = 0;
  for (i = start; i < end ; i++) {
    mysum += (x[i] * y[i]);
  }

  // dotstr.sum is the shared data, must be protected.
  pthread_mutex_lock (&mutexsum);
  dotstr.sum += mysum;
  pthread_mutex_unlock (&mutexsum);

  pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
  long i;
  double *a, *b;
  void *status;
  pthread_attr_t attr;

  // initialize data
  a = (double*)malloc(NUMTHREADS * VECLEN * sizeof(double));
  b = (double*)malloc(NUMTHREADS * VECLEN * sizeof(double));
  for (i = 0; i < VECLEN * NUMTHREADS; i++) {
    a[i] = b[i] = 1.0;
  }
  dotstr.veclen = VECLEN;
  dotstr.a = a;
  dotstr.b = b;
  dotstr.sum=0;

  // initialize mutex
  pthread_mutex_init(&mutexsum, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // create thread to calc products
  for (i = 0; i < NUMTHREADS; i++) {
    pthread_create(&call_thread[i], &attr, dotprod, (void*)i);
  }

  pthread_attr_destroy(&attr);
  for (i = 0; i < NUMTHREADS; i++) {
    pthread_join(call_thread[i], &status);
  }
  printf("Sum = %f\n", dotstr.sum);

  free(a); free (b);
  pthread_mutex_destroy(&mutexsum);
  pthread_exit(NULL);
}
