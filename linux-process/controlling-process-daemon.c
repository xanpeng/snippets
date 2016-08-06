// daemon的好处：
// 1、父进程是init进程，不受terminal关闭影响
// 2、不被Ctrl-C杀死
// 3、...
//
// http://stackoverflow.com/questions/3095566/linux-daemonize
// http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
// http://www.danielhall.me/2010/01/writing-a-daemon-in-c/

#include <stdlib.h>
#include <unistd.h>

int main(void) {
  pid_t pid, sid;

  // fork创建子进程成功后，父进程退出，从而子进程（也就是待实现的daemon进程）成为init进程的子进程；
  // 这么做的目的有：
  //  1. 父进程退出，shell感知这一信息，从而继续下一prompt，让子进程在后台"自由"运行；
  //  2. 子进程不会是进程组leader，这是setsid()的前提，进程组leader执行setsid()不会成功
  //    （如果可以，leader sid变了，但进程组里其他进程的sid还没变，从而违背session-进程组的抽象设定——同一进程组的所有进程sid相同）
  pid = fork();
  if (pid < 0) { exit(EXIT_FAILURE); }
  if (pid > 0) { exit(EXIT_SUCCESS); }

  // 创建一个新的session，不关联任何controlling terminal
  sid = setsid();
  if (sid < 0) { exit(EXIT_FAILURE); }
  // 如果进程不再使用terminal device，则一切无碍；但如果进程后续可能使用terminal device，则需要防止它成为controlling terminal;
  // (the controlling terminal is established when the session leader first opens a terminal that is not already the controlling terminal for a session)
  // (一旦有controlling terminal，terminal断连时，内核就会给controlling process，也就是session leader，也就是daemon进程发SIGHUP——这正是我们要避免的)
  //      1. open(terminal-device)时指定O_NOCTTY选项；
  //      2. setsid()之后，再fork()新的子进程，然后父进程退出，从而保证子进程不会是session leader；

  // 确保daemon进程有足够的文件操作权限
  umask(0);

  // 打开log文件

  // 避免不能umount占据的文件系统
  if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }

  // 关闭继承下来的、不被使用的文件。0,1,2就是典型
  // close(STDIN_FILENO);
  // close(STDOUT_FILENO);
  // close(STDERR_FILENO);
  int maxfd = sysconf(_SC_OPEN_MAX);
  if (maxfd == -1) { ...}
  for (fd = 0; fd < maxfd; ++fd) close(fd);

  // 打开/dev/null，并通过dup2()将0、1、2都关联到/dev/null，以防进程可能使用fd 1/2打开文件
  close(STDIN_FILENO);
  fd = open("/dev/null", O_RDWR);
  if (fd != STDIN_FILENO) return -1;  // fd应该是0
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) return -1;
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) return -1;

  while (1) { sleep(30); }
  exit(EXIT_SUCCESS);
}

/******************** 第二种实现方法 ****************************/
/*
#include <syslog.h>
#include <unistd.h>

int main() {
  daemon(1, 1);

  long long counter = 0;
  while (1) {
    sleep(2);
    syslog(LOG_DEBUG, "daemon example log %lld", ++counter);
  }

  return 0;
}
*/
