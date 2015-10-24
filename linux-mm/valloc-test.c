#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main()
{
	const int nr_bytes = 128;
	void *data = valloc(nr_bytes);
	if (!data)
		handle_error("failed to valloc");
	memset(data, 0, nr_bytes);
	return 0;
}

/*
# ltrace ./valloc-test
(0, 0, 253952, -1, 0x1f25bc2)                                                                         = 0x7f5cb3676160
__libc_start_main(0x400594, 1, 0x7fff4e0159f8, 0x400600, 0x4005f0 <unfinished ...>
valloc(128, 0x7fff4e0159f8, 0x7fff4e015a08, 0, 0x7f5cb3450300)                                        = 0x76b000
memset(0x76b000, '\000', 128)                                                                         = 0x76b000
+++ exited (status 0) +++

# strace ./valloc-test
execve("./valloc-test", ["./valloc-test"], [<34 vars>]) = 0
brk(0)                                  = 0x2107000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fef301cf000
函数签名：void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
open("/etc/ld.so.cache", O_RDONLY)      = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=77079, ...}) = 0
mmap(NULL, 77079, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7fef301bc000  // ld.so.cache的大小就是77079
close(3)                                = 0
open("/lib64/libc.so.6", O_RDONLY)      = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0p\356\1\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1921176, ...}) = 0
mmap(NULL, 3750152, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fef2fc1d000
mprotect(0x7fef2fda7000, 2097152, PROT_NONE) = 0
函数签名：int mprotect(void *addr, size_t len, int prot);  // 修改prot

mmap(0x7fef2ffa7000, 20480, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x18a000) = 0x7fef2ffa7000
mmap(0x7fef2ffac000, 18696, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7fef2ffac000
close(3)                                = 0
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fef301bb000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fef301ba000
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fef301b9000
arch_prctl(ARCH_SET_FS, 0x7fef301ba700) = 0
mprotect(0x7fef2ffa7000, 16384, PROT_READ) = 0
mprotect(0x7fef301d0000, 4096, PROT_READ) = 0
munmap(0x7fef301bc000, 77079)           = 0
brk(0)                                  = 0x2107000
brk(0x2129000)                          = 0x2129000
exit_group(0)                           = ?
 */
