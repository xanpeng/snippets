#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>


void *thread_func(void *data)
{
	int rand_size;
	int rand_memset_size;
	int rand_sleep_time;
	char *array = NULL;
	int seed = *((int*) data);

	srandom(seed);

	while (1) {
#define MAX_MEM_SIZE 512 * 1024
// #define MAX_MEM_SIZE 16 * 1024 * 1024
		rand_size = random() % MAX_MEM_SIZE;
		rand_size = rand_size ? rand_size : 1;
		rand_memset_size = random() % rand_size;
		rand_sleep_time = random() % 7;

		array = (char*) malloc(rand_size);
		memset(array, 'A', rand_memset_size);
		fprintf(stderr, "%d: malloc %d used %d\n", seed, rand_size, rand_memset_size);

		sleep(rand_sleep_time);

		free(array);
		array = NULL;
	}
}

// 1) with mallopt, monitor rss,
// 2) without mallopt, monitor rss,
// 3) use tcmalloc
int main()
{
	const int nr_threads = 512;
	pthread_t threads[nr_threads];
	int i;
	int ret;
	int nr_running_threads = 0;

	daemon(1, 1);
	mallopt(M_MMAP_THRESHOLD, 512 * 1024);

	for (i = 0; i < nr_threads; ++i) {
		ret = pthread_create(&threads[i], NULL, thread_func, (void*)&i);
		if (ret)
			fprintf(stderr, "pthread_create %d failed with %d\n", i, ret);
		else
			nr_running_threads++;
	}

	printf("join %d threads\n", nr_running_threads);
	for (i = 0; i < nr_threads; ++i) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
