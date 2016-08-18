#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if (argc > 1) {
        int fd = open(argv[1], O_WRONLY);
        if(fd == -1) {
            printf("Unable to open the file\n");
            exit(1);
        }
        static struct flock lock;

        lock.l_type = F_WRLCK;
        lock.l_start = 0;
        lock.l_whence = SEEK_SET;
        lock.l_len = 0;
        lock.l_pid = getpid();

        // 根据上面提到的kernel文档，mandatory lock只能使用fcntl()/lockf()，
        // 用上面的python flock试过，的确没有效果
        int ret = fcntl(fd, F_SETLKW, &lock); 
        printf("Return value of fcntl:%d\n",ret);
        if(ret==0) {
            while (1) {
                scanf("%c", NULL);
            }
        }
    }
}

/*
# dd if=/dev/zero of=/tmp/4gdisk bs=1M count=4096
# modprobe loop && losetup /dev/loop0 /tmp/4gdisk
# mkfs.ext4 /dev/loop0
# mkdir /xdata && mount -o mand /dev/loop0 /xdata
# cd /xdata && touch advisory_file && touch mandatory_file
# chmod g+s,g-x mandatory_file

# (测试advisory lock)
# ./filelocker advisory_file
 (等待输入)
# (在另一个shell下面) ls >> advisory_file
 (执行成功)

# (测试mandatory lock）
# ./filelocker mandatory_file
(等待输入)
# (在另一个shell下面) ls >> mandatory_file
(执行没有立即成功，而是被阻止，相信也没有超时设置)
*/
