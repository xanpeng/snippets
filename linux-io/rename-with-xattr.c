#include <stdio.h>
#include <stdlib.h>
#include <attr/xattr.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	const char *tmppath = "/tmp/filewithxattr.tmp";
	const char *path = "/tmp/filewithxattr";
	char data[4096];
	int fd;
	uint32_t version = 1023;

	fd = open(tmppath, O_CREAT|O_EXCL|O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (fd < 0)
		handle_error("failed to open");
	
	if (write(fd, data, 4096) < 0)
		handle_error("failed to write");

	if (setxattr(tmppath, "user.obj.version", &version, 4, 0) < 0)
		handle_error("failed to setxattr");

	if (rename(tmppath, path) < 0)
		handle_error("failed to rename");

	version++;

	if (getxattr(path, "user.obj.version", &version, 4) != 4)
		handle_error("failed to getxattr");

	printf("%s user.obj.version=%d\n", path, version); // /tmp/filewithxattr user.obj.version=1023

	return 0;
}
