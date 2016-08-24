#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>

// malloc stat探测：mallinfo(3), malloc_info(3), malloc_stats(3), mtrace(3)/muntrace(3), mcheck(3), malloc_hook(3)
static void my_init_hook(void);
static void *my_malloc_hook(size_t size, const void *caller);
static void *(*old_malloc_hook)(size_t, const void *);

// uncomment to hook
// void (*__malloc_initialize_hook)(void) = my_init_hook; // overide initializing hook from glibc

static void my_init_hook(void)
{
    old_malloc_hook = __malloc_hook;
    __malloc_hook = my_malloc_hook;
}

static void *my_malloc_hook(size_t size, const void *caller)
{
    void *mem;

    __malloc_hook = old_malloc_hook;
    mem = malloc(size);
    old_malloc_hook = __malloc_hook;
    // malloc_stats();
    /* malloc_stats() output like this:
     Arena 53:
     system bytes     =    1208320
     in use bytes     =    1206592
     */

    malloc_info(0, stdout);
    printf("\n\n");

    __malloc_hook = my_malloc_hook;
    return mem;
}

static inline uint64_t clock_get_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000LL+ (uint64_t)ts.tv_nsec/1000000;
}

// http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
unsigned random_at_most(long max)
{
    unsigned long nr_bins = (unsigned long)max + 1;
    unsigned long nr_rand = (unsigned long)RAND_MAX + 1;
    unsigned long bin_size = nr_rand / nr_bins;
    unsigned long defect = nr_rand % nr_bins;
    long x;

    do {
        x = random();
    } while (nr_rand - defect <= (unsigned long)x);

    return (unsigned)(x / bin_size);
}

unsigned random_io_latency(void)
{
  const unsigned us_min = 5 * 1000;
  const unsigned us_max = 30 * 1000;
  return us_min + random_at_most(us_max - us_min);
}

const unsigned MB = 1024 * 1024;
unsigned g_max_ms = 0;

void alloc_and_hold(int nr_bytes, int us_hold)
{
    void *mm;
    uint64_t ts_start, ts_diff;

    ts_start = clock_get_ms();
    mm = malloc(nr_bytes);
    ts_diff = clock_get_ms() - ts_start;
    if (ts_diff >= g_max_ms)
        fprintf(stdout, "alloc_and_hold alloc(%dKB) used %dms, will hold for %ums\n", nr_bytes / 1024, ts_diff, us_hold / 1000);
    memset(mm, 0, nr_bytes);
    //for (i = 0; i < nr_bytes; i++) mm[i] = 'a';
    //mm[0] = 'a', mm[nr_bytes-1] = 'a';

    usleep(us_hold);

    ts_start = clock_get_ms();
    free(mm);
    mm = NULL;
    ts_diff = clock_get_ms() - ts_start;
    // if (ts_diff >= g_max_ms)
    //    fprintf(stdout, "alloc_and_hold free(%d KB) used %dms\n", nr_bytes / 1024, ts_diff);
}

void *thread_io_alloc_mem(void *arg)
{
    while (1) {
        alloc_and_hold(random_at_most(2 * MB), random_io_latency());
    }
}

void *thread_recovery_alloc_mem(void *arg)
{
    while (1) {
        alloc_and_hold(4 * MB, random_io_latency());
    }
}

void create_threads(int nr_threads, int for_io)
{
    int ret, cnt = 0;
    pthread_t thread;
    void *thread_func = for_io ? thread_io_alloc_mem : thread_recovery_alloc_mem;

    while (cnt < nr_threads) {
        ret = pthread_create(&thread, NULL, thread_func, NULL);
        if (ret) {
            fprintf(stderr, "failed to create thread\n");
            exit(1);
        }
        ++cnt;
    }
}

int main(int argc, char *argv[])
{
    int nr_io_threads, nr_recovery_threads;

    if (argc != 4) {
        fprintf(stderr, "Usage: mulalloc nr_io_threads nr_recovery_threads max_ms\n");
        exit(1);
    }

    mallopt(M_MMAP_THRESHOLD, 512 * 1024);
    nr_io_threads = atoi(argv[1]);
    nr_recovery_threads = atoi(argv[2]);
    g_max_ms = atoi(argv[3]);
    create_threads(nr_io_threads, 1);
    create_threads(nr_recovery_threads, 0);

    sleep(60 * 60 * 60);
    return 0;
}

/*
gcc mulalloc.c -o mulalloc -lpthread -lrt
for i in {1..12}; do mulalloc 400 0 2000 &  # 400线程分配随机大小[1B~2MB]，0线程分配4M，超过2s打印

0）计时不考虑memset，仅记录malloc的耗时，发现仍然有大量超时的。这说明glibc malloc耗时多，以及mmap申请内存耗时多。
alloc_and_hold alloc(1029 KB) used 3263ms
alloc_and_hold alloc(197 KB) used 21867ms
alloc_and_hold alloc(1804 KB) used 2448ms
alloc_and_hold alloc(111 KB) used 3246ms
alloc_and_hold alloc(1004 KB) used 19728ms
alloc_and_hold alloc(1361 KB) used 2286ms
alloc_and_hold alloc(1997 KB) used 2453ms
alloc_and_hold alloc(1581 KB) used 2283ms
alloc_and_hold alloc(914 KB) used 58210ms


1）在注释mallopt时，malloc/free耗时>2s的特别多。每进程rss=1.2G，系统剩余内存15G，观测buddyinfo每个list都有。
系统整体响应慢，如执行ps感觉明显卡顿，ssh登录特别慢，为什么？perf top发现是因为大量mmap/munmap导致：
 42.55%  libc-2.12.so                        [.] __memset_sse2
 13.23%  [kernel]                            [k] __alloc_pages_nodemask
  8.00%  [kernel]                            [k] clear_page_c_e
  7.37%  [kernel]                            [k] _spin_lock
  1.89%  [kernel]                            [k] __mem_cgroup_commit_charge
  1.82%  [kernel]                            [k] page_fault
  1.68%  [kernel]                            [k] __mem_cgroup_uncharge_common
  1.48%  libcrypto.so.1.0.1e                 [.] 0x000000000008c031
  1.42%  [kernel]                            [k] unmap_vmas

2）如果控制alloc内存大小(0, 128KB)，不使用mallopt，同时避开mmap，预计系统响应不会慢。
实测证明如此，strace -f -q看不到mmap，perf top输出：
 27.08%  libc-2.12.so                 [.] __memset_sse2
  5.11%  [kernel]                     [k] _spin_lock
  3.68%  [kernel]                     [k] clear_page_c_e
  2.71%  [kernel]                     [k] __alloc_pages_nodemask
  2.44%  [kernel]                     [k] page_fault
  2.38%  libcrypto.so.1.0.1e          [.] 0x000000000015475d
  2.37%  libruby.so.1.8.7             [.] 0x000000000009bd20

3）如果mallopt(4M)，避开mmap，预计系统响应不会慢。即使进程自己申请内存可能慢。
然而并没有什么卵用，系统整体响应依然慢，perf top输出：
 52.60%  libc-2.12.so          [.] __memset_sse2
 16.24%  [kernel]              [k] __alloc_pages_nodemask
 10.10%  [kernel]              [k] clear_page_c_e
  2.21%  [kernel]              [k] __mem_cgroup_commit_charge
  2.04%  [kernel]              [k] page_fault
  1.45%  [kernel]              [k] __mem_cgroup_uncharge_common
  1.33%  [kernel]              [k] unmap_vmas
  1.04%  [kernel]              [k] list_del
  0.72%  [kernel]              [k] free_pcppages_bulk
  0.64%  [kernel]              [k] down_read_trylock



根据上面初步测试，设计几组实验，根据是否mallopt，申请内存的大小范围来祝贺，计时不考虑memset，观察系统响应快慢，strace输出，perf top的输出。

*/
