// pthread stack size can be larger than system limit (ulimit -s)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
 
void* func(void* threadid)
{
  long tid = (long) threadid;
  printf("hello world, thread #%ld!\n", tid);
  pthread_exit(NULL);
}
 
int main()
{
  pthread_attr_t attr;
  pthread_t threadid;
  size_t stacksize;
  struct rlimit rlim;
  int ret;
  int MB = 1024 * 1024;
 
  pthread_attr_init(&attr);
  pthread_attr_getstacksize(&attr, &stacksize);
  getrlimit(RLIMIT_STACK, &rlim);
  printf("stack limit: %zd MB\n", (size_t) rlim.rlim_cur / MB);
  printf("stack size: %zd MB\n", stacksize / MB);
 
  ret = pthread_attr_setstacksize(&attr, stacksize * 20);
  if (ret)
  {
    perror(strerror(ret));
    return -1;
  }
  pthread_attr_getstacksize(&attr, &stacksize);
  printf("new stacksize: %zd MB\n", stacksize / MB);
 
  ret = pthread_create(&threadid, &attr, func, (void*)threadid);
  if (ret)
  {
    perror(strerror(ret));
    return -1;
  }
 
  pthread_exit(NULL);
}
