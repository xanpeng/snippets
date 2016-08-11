///////////////////////////////////////////
// main.c
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	int fd = open("test.txt", O_CREAT | O_RDWR | O_APPEND);
	if (fd == -1) {
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	fcntl(fd, F_SETFD, 1);

	pid_t pid = fork();
	if (pid == 0)
		execl("another_program", "./another_program", &fd, NULL);
	wait(NULL);

	char *msg = "xxxxxxxxxxxxxxxxxxxxx\n";
	write(fd, msg, strlen(msg));
	close(fd);

	return 0;
}

///////////////////////////////////////////
// another_program.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int fd = *argv[1];
    printf("[another_program] fd: %d\n", fd);
    char *msg = "ooooooooooooooooooooo\n";

    int ret = write(fd, msg, strlen(msg));
    if (-1 == ret)
        printf("%s\n", strerror(errno));
    printf("written %d bytes\n", ret);

    close(fd);

    return 0;
}


///////////////////////////////////////////
// execute result
// 通过fcntl(fd, F_SETFD,1)设置close_on_exec
// 没有设置close_on_exec的时候，B从A继承了打开的文件，是可以写成功的。设置了之后，B是不能写成功的（应该是vfs_write之前就发现fd是非法的，从而写不能成功，且报错-EBADF）。
/*
// set close_on_exec
# ./main
[another_program] fd: 3
Bad file descriptor
written -1 bytes

# cat test.txt
xxxxxxxxxxxxxxxxxxxxx

// not set close_on_exec
# ./main
[another_program] fd: 3
written 22 bytes

# cat test.txt
ooooooooooooooooooooo
xxxxxxxxxxxxxxxxxxxxx
*/
