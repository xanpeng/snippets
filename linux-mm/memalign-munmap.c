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
