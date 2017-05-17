#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <poll.h>

void *recv_fn(void *data)
{
	int fd = *(int *)data;
	printf("[thread] about to read from fd=%d...\n", fd);

	// test poll + main-thread-shutdown
	{
		int pollret;
		struct pollfd pf[1];
		pf[0].fd = fd;
		pf[0].events = POLLIN;

		pollret = poll(pf, 1, 5 * 1000);
		if (pollret < 0) {
			printf("[thread] poll <0\n");
			return NULL;
		} else if (pollret == 0) {
			printf("[thread] poll timeout\n");
			return NULL;
		} else if (pf[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
			printf("[thread] poll revents=%d\n", pf[0].revents);
			return NULL;
		}
	}

	// test read + main-thread-shutdown
	char buf[1024] = {0};
	int n = read(fd, buf, 1023);
	if (n > 0) {
		printf("[thread] read n=%d, msg=%s\n", n, buf);
	}
	if (n < 0) {
		printf("[thread] ERROR: read error err=%d (%m)\n", errno);
	}
	if (n == 0)
		printf("[thread]: read 0\n");

	return NULL;
}

int main(int argc, char **argv)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(lfd > 0);
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in saddr;
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons( 9000 );

    int rc = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
	assert(rc == 0);
    rc = listen(lfd, 128);
	assert(rc == 0);

    printf("[main] lfd=%d\n", lfd);
    int cfd = accept(lfd, (struct sockaddr*)NULL, NULL);
	assert(cfd > 0);
	printf("[main] accepted cfd=%d\n", cfd);

	struct timeval timeout;
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;
	setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	pthread_t tid;
	rc = pthread_create(&tid, NULL, recv_fn, &cfd);

	printf("[main] sleep 1...\n");
	sleep(1);

	// testing effect of close(2) vs shutdow(2)
	// close(cfd);
	shutdown(cfd, SHUT_RDWR);
	printf("[main] fd destroyed...\n");

	rc = pthread_join(tid, NULL);
	assert(rc == 0);

	// close(cfd);
	close(lfd);
	printf("[main] exiting...\n");
    return 0;
}
