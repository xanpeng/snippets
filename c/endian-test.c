// make endian-test
#include <stdio.h>
#include <limits.h>  // CHAR_BIT
#include <inttypes.h> // PRIu64

struct compv {
  union {
    struct {
      uint32_t u32v0;
      uint32_t u32v1;
    };
    uint64_t bits;
  };
};

/**
 * test endianness of current machine 
 * mem: 0D 0C 0B 0A //big endian 
 * mem: 0A 0B 0C 0D //little endian 
 * V=0x0D0C0B0A
 * endianness是计算机一个WORD中byte的排序方式。
 * WORD: https://en.wikipedia.org/wiki/Word_(computer_architecture)，CPU或指令集处理的单元数据大小。
 * 但实际上，存储一个uint32_t，按照4字节去按照endianness排序，
 *           存储一个uint64_t，按照8字节去按照endianness排序。
 * 下面main()中很多测试，都是遵循这个事实。
 */
static void check_endian() {
  int v = 0x0D0C0B0A;
  char c = *(char*)&v;

  printf("current machine is probably %d bits\n", (int)(CHAR_BIT * sizeof(void *)));  // 64 bits
  printf("current machine long int size is %d bits\n", (int)(sizeof(long int)));      // 8 bits
  printf("current machine long long size is %d bits\n", (int)(sizeof(long long int))); // 8 bits
  if (c == 0x0A)  
    printf("current machine is little endian\n");
  else
    printf("current machine is big endian\n");
}

static void show_mem_rep(char *start, int n) {
  int i;

  for (i = 0; i < n; ++i)
    printf(" %.2x", start[i]);
  printf("\n");
}

int main() {
  uint64_t u64v = 0x0102030405060708;
  printf("mem rep of u64v: ");
  show_mem_rep((char*)&u64v, sizeof(u64v)); // 08 07 06 05 04 03 02 01

  struct compv cid;
  cid.u32v0 = 0x65;
  cid.u32v1 = 0x7b;

  printf("u32v0 u32v1: %u %u\n", cid.u32v0, cid.u32v1);     // u32v0 u32v1: 101 123
  printf("     in hex: %08x %08x\n", cid.u32v0, cid.u32v1); //      in hex: 00000065 0000007b

  check_endian();  // current machine is little endian

  printf("mem rep of compv.u32v0: ");
  show_mem_rep((char*)&cid.u32v0, sizeof(cid.u32v0)); // 65 00 00 00
  printf("mem rep of compv.u32v1: ");
  show_mem_rep((char*)&cid.u32v1, sizeof(cid.u32v1)); // 7b 00 00 00

  printf("       llu bits: %llu\n", cid.bits);        // 528280977509
  // PRIu64是uint64_t更合理的打印方式，更多类似宏：http://en.cppreference.com/w/c/types/integer
  printf("    PRIu64 bits: %" PRIu64 "\n", cid.bits); // 528280977509
  printf("    bits in hex: %llx\n", cid.bits);        // 7b00000065
  printf("bits hex prefix: %016llx\n", cid.bits);     // 0000007b00000065

  printf("\nmem rep of compv.bits: ");
  show_mem_rep((char*)&cid.bits, sizeof(cid.bits));   // 65 00 00 00 7b 00 00 00

  // 32位系统和64位系统，自然值=后32bit小端转大端 接 先32bit小端转大端
  printf("0x650000007b000000=%llu\n", (uint64_t)0x650000007b000000); // 7277816999894319104
  printf("0x000000650000007b=%llu\n", (uint64_t)0x000000650000007b); // 433791697019
  printf("0x7b00000065=%llu\n", (uint64_t)0x7b00000065);             // 528280977509
  printf("0x0000007b00000065=%llu\n", (uint64_t)0x0000007b00000065); // 528280977509

  return 0;
}
