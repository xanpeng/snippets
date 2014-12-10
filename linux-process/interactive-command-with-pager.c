/**
  非常简单，只有三个小亮点：
  1、getenv("PAGER")
  2、pager效果是通过“管道+less”做出来的
  3、signal(SIGPIPE, SIG_IGN)，参考http://stackoverflow.com/questions/6824265/sigpipe-broken-pipe

  效果
  1、
# ./a.out
hell

2、
I don't really care what your command is
lines 1-1/1 (END)

3、
# ./a.out
hell
hell0
*/

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

/** copy from e2fsprogs-1.32/debugfs/util.c */
static FILE *open_pager(int interactive)
{
  FILE *outfile = NULL;
  const char *pager = getenv("PAGER"); // # echo $PAGER

  if (interactive) {
    signal(SIGPIPE, SIG_IGN);
    if (pager) {
      if (strcmp(pager, "__none__") == 0)
        return stdout;
    }
    else
      pager = "more";

    outfile = popen(pager, "w");
  }

  return (outfile ? outfile : stdout);
}

static void close_pager(FILE *stream)
{
  if(stream && stream != stdout)
    pclose(stream);
}

static void do_command(char *line)
{
  FILE *out;

  out = open_pager(1);
  fprintf(out, "I don't really care what your command is\n");
  close_pager(out);
}

int main()
{
  char *line;

  while (1) {
    line = readline(NULL);
    if (line) {
      do_command(line);
    }
  }
}
