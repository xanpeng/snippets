#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void handler(int sig) { }    // 忽视SIGHUP等信号

int main(int argc, char *argv[])
{
    pid_t child_pid;
    struct sigaction sa;

    setbuf(stdout, NULL);       /* Make stdout unbuffered */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0 && argc > 1)
        if (setpgid(0, 0) == -1) {       /* Move to new process group */
            perror("setpgid");
            exit(EXIT_FAILURE);
        }

    printf("PID=%ld; PPID=%ld; PGID=%ld; SID=%ld\n", (long) getpid(), (long) getppid(), (long) getpgrp(), (long) getsid(0));

    alarm(60);                  /* An unhandled SIGALRM ensures this process will die if nothing else terminates it */
    for(;;) {                   /* Wait for signals */
        pause();
        printf("%ld: caught SIGHUP\n", (long) getpid());
    }
}

/*
// 操作说明
1. ./catch-sighup > samegroup.log 2>&1 &    // 没有多余参数，表示catch-sighup进程以及fork的子进程属于同一进程组
2. ./catch-sighup x > diffgroup.log 2>&1 &  // 有多余参数，表示catch-sighup进程fork的子进程通过setpgid()进入另一进程组
3. kill -SIGHUP 721                         // 721事先确定，是运行程序的shell的pid
(terminal窗口消息，新开窗口执行如下命令)
4. cat samegroup.log 
PID=1087; PPID=721; PGID=1087; SID=721      // 1087是./catch-sighup进程，1088是它fork的子进程 
PID=1088; PPID=1087; PGID=1087; SID=721
1087: caught SIGHUP                         // 这两个进程都属于进程组1087，是shell创建的进程组
1088: caught SIGHUP                         // 因此都因SIGHUP shell而接收到SIGHUP
5. cat diffgroup.log 
PID=1116; PPID=721; PGID=1116; SID=721      // 1116是./catch-sighup进程，1117是它fork的子进程
PID=1117; PPID=1116; PGID=1117; SID=721     // 1117被setpgid()过，因而属于不同的进程组，且这个进程组不属于shell创建的
1116: caught SIGHUP                         // 从而只有1116接收到SIGHUP
*/
