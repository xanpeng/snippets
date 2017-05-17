#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int sockfd = 0, n = 0;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("ERROR: Could not create socket \n");
		return 1;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(9000);

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
		printf("ERROR: inet_pton error occured\n");
		return 1;
	}

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("ERROR: Connect Failed \n");
		return 1;
	}

	// cooperate with ./recv, testing shutdown(2) vs close(2)
	printf("connected. sleeping 5...\n");
	sleep(10);

	time_t ticks = time(NULL);
	char buf[1024];
	memset(buf, '0',sizeof(buf));
	snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
	write(sockfd, buf, strlen(buf));

	printf("sent %s\n", buf);

	// sleep(5);

	close(sockfd);
	return 0;
}
