#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

int glob = 1;
void err_sys(const char* fmt, ...) {
	va_list ap;
	fprintf(stderr, fmt, ap);
	exit(1);
}

int main() {
	int var;
	pid_t pid;

	var = 1;
	printf("before vfork\n");

	if ((pid = vfork()) < 0)
		err_sys("vfork error");
	else if (pid == 0) { /* child */
		/*
		 * Because vfork makes child to share address space with parent,
		 * the child process will increment the glob variable in parent space;
		 */
		printf("modifying variables in parent address space.\n");
		glob++;
		var++;

		/*
		 * Compared with exit(), _exit() won't flush io and close the stdout. Because we
		 * are sharing address space with parent, we must use _exit() so in following
		 * parent printf() call, the parent could output to stdout. If we use exit here
		 * instead, the printf() in parent won't print anything.
		 */
		_exit(0);
	}

	/* parent */
	/*
	 * if we use exit() instead of _exit() in child process, printf won't output anything,
	 * because stdout is already closed.
	 */
	printf("pid = %d, glob = %d, var = %d\n", getpid(), glob, var);
	exit(0);
}

/*****************
  before vfork
  modifying variables in parent address space.
  pid = 17282, glob = 2, var = 2
 *****************/
