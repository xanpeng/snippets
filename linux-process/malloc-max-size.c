// find the max size malloc() can apply for
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// "cat /proc/meminfo" get this:
// MemTotal: 25111808 kb
int main(void)
{
  int i;
  void* memptr;
  for (i = 1023; i >=0; i--)
  {
    memptr = malloc(25111808);

    // 情况一：没有操作实际内存，毫无压力
    // memset(memptr, 0, 25111808);

    // 情况二：操作分配的内存，压力很大，不过没有挂掉，应该是swap分区"立功"了
    memset(memptr, 0, 25111808);

    // TODO: 情况三，获取所有swap分区和文件，是否可以 disable all swap? 然后再执行 malloc 和 memset.
  }

  return 0;
}
