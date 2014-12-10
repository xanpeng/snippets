// daemon的好处：
// 1、父进程是init进程，不受terminal关闭影响
// 2、不被Ctrl-C杀死
// 3、...
//
// http://stackoverflow.com/questions/3095566/linux-daemonize
// http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
// http://www.danielhall.me/2010/01/writing-a-daemon-in-c/

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
