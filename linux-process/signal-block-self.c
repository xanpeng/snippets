/*
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
sig：指定信号。
act：指定处理函数等。
act.sa_mask：处理这个信号时，屏蔽其它哪些信号。
oact：非NULL值表示这次处理完之后，恢复到oact保存的原来的处理函数。

另外有一个signal()函数也能捕获信号，但它没有sigaction可靠，功能也不完备，不推荐使用。  
*/

/* Example of using sigaction() to setup a signal handler with 3 arguments including siginfo_t. */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static void hdl (int sig, siginfo_t *siginfo, void *context) {
	printf ("Sending PID: %ld, UID: %ld\n", (long)siginfo->si_pid, (long)siginfo->si_uid);
}

int main (int argc, char *argv[]) {
	struct sigaction act;
	memset (&act, '\0', sizeof(act));
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = &hdl;
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}

	while (1)
		sleep (10);

	return 0;
}
