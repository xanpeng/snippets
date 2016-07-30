#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    void *arr;
    printf("posix_memalign: %d\n", posix_memalign(&arr, 4096, 4096 * 5));
    printf("address of arr: %p\n", arr);
    printf("address of arr[4096]: %p\n", arr + 4096);
    printf("address of arr[4096*3]: %p\n", arr + 4096*3);

    puts("before munmap...");
    sleep(30);

    // 分配页0、1、2、3、4，释放1、3.
    printf("munmap: %d\n", munmap(arr + 4096, 4096));
    printf("munmap: %d\n", munmap(arr + 4096*3, 4096));

    puts("after munmap...");
    sleep(30);
    return 0;
}

/*
# ./mmap
posix_memalign: 0
address of arr: 0x603000
address of arr[4096]: 0x604000
address of arr[4096*3]: 0x606000
before munmap...
munmap: 0
munmap: 0
after munmap...
*/

/*
# cat /proc/16456/maps  --> munmap之前
起止地址    permissions    对应文件的offset(无则为0)    device    inode(无则为0)    pathname
00400000-00401000 r-xp 00000000 08:07 97922                              /home/xan/lab/mmap
00600000-00601000 r--p 00000000 08:07 97922                              /home/xan/lab/mmap
00601000-00602000 rw-p 00001000 08:07 97922                              /home/xan/lab/mmap
00602000-00629000 rw-p 00000000 00:00 0                                  [heap]
7fa00248e000-7fa0025e2000 r-xp 00000000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0025e2000-7fa0027e2000 ---p 00154000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e2000-7fa0027e6000 r--p 00154000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e6000-7fa0027e7000 rw-p 00158000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e7000-7fa0027ec000 rw-p 00000000 00:00 0
7fa0027ec000-7fa00280b000 r-xp 00000000 08:01 180579                     /lib64/ld-2.11.1.so
7fa0029d7000-7fa0029da000 rw-p 00000000 00:00 0
7fa002a08000-7fa002a0a000 rw-p 00000000 00:00 0
7fa002a0a000-7fa002a0b000 r--p 0001e000 08:01 180579                     /lib64/ld-2.11.1.so
7fa002a0b000-7fa002a0c000 rw-p 0001f000 08:01 180579                     /lib64/ld-2.11.1.so
7fa002a0c000-7fa002a0d000 rw-p 00000000 00:00 0
7fffa3def000-7fffa3e03000 rw-p 00000000 00:00 0                          [stack]
7fffa3e19000-7fffa3e1a000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]

# cat /proc/16456/maps  --> munmap之后
00400000-00401000 r-xp 00000000 08:07 97922                              /home/xan/lab/mmap
00600000-00601000 r--p 00000000 08:07 97922                              /home/xan/lab/mmap
00601000-00602000 rw-p 00001000 08:07 97922                              /home/xan/lab/mmap
00602000-00604000 rw-p 00000000 00:00 0                                  [heap]     --> 不同之处
00605000-00606000 rw-p 00000000 00:00 0                                  [heap]     --> 不同之处
00607000-00629000 rw-p 00000000 00:00 0                                  [heap]
7fa00248e000-7fa0025e2000 r-xp 00000000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0025e2000-7fa0027e2000 ---p 00154000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e2000-7fa0027e6000 r--p 00154000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e6000-7fa0027e7000 rw-p 00158000 08:01 180672                     /lib64/libc-2.11.1.so
7fa0027e7000-7fa0027ec000 rw-p 00000000 00:00 0
7fa0027ec000-7fa00280b000 r-xp 00000000 08:01 180579                     /lib64/ld-2.11.1.so
7fa0029d7000-7fa0029da000 rw-p 00000000 00:00 0
7fa002a08000-7fa002a0a000 rw-p 00000000 00:00 0
7fa002a0a000-7fa002a0b000 r--p 0001e000 08:01 180579                     /lib64/ld-2.11.1.so
7fa002a0b000-7fa002a0c000 rw-p 0001f000 08:01 180579                     /lib64/ld-2.11.1.so
7fa002a0c000-7fa002a0d000 rw-p 00000000 00:00 0
7fffa3def000-7fffa3e03000 rw-p 00000000 00:00 0                          [stack]
7fffa3e19000-7fffa3e1a000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
*/
