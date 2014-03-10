#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "cexception.h"

#define mark printf("[DEBUG] %s:%d\n", __func__, __LINE__)

void func_throw()
{
    mark;
    Throw(1);
    mark;
}

/*
 * cexception cannot work with multi-thread right now.
 * More work need to be done.
 * See https://github.com/ThrowTheSwitch/CException/blob/master/docs/CExceptionSummary.pdf
 *  and search "task" for more information, also other limitations.
 */
void *thread_worker(void *data)
{
    CEXCEPTION_T e;
    char *message = (char*)data;
    Try {
        mark;
        func_throw();
        mark;
    }
    Catch(e)
        printf("exception: %d\n", e);
    puts(message);
}

/* gcc threads-with-cexception.c cexception.c -o threads-exception */
int main()
{
    pthread_t thread1, thread2;
    int ret1, ret2;

    ret1 = pthread_create( &thread1, NULL, thread_worker, (void*) "thread1-msg");
    ret2 = pthread_create( &thread2, NULL, thread_worker, (void*) "thread2-msg");

    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);

    printf("Thread 1 returns: %d\n", ret1);
    printf("Thread 2 returns: %d\n", ret2);
    return 0;
}
