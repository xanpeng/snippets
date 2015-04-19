#include <stdio.h>
#include <inttypes.h>

struct chunkid {
  union {
    struct {
      uint32_t volumeid;
      uint32_t index;
    };
    uint64_t bits;
  };
};

void check_endian() {
  unsigned int i = 1;
  char *c = (char*)&i;
  if (*c)
    printf("little endian\n");
  else
    printf("big endian\n");
}

void show_mem_rep(char *start, int n) {
  int i;
  for (i = 0; i < n; ++i)
    printf(" %.2x", start[i]);
}

int main() {
  uint32_t volid = 101;
  uint32_t index = 123;
  struct chunkid cid;
  cid.volumeid = volid;
  cid.index = index;


  printf("      chunkid: (%u %u)\n", cid.volumeid, cid.index);      // 00000065
  printf("       in hex: (%08x %08x)\n", cid.volumeid, cid.index);  // 0000007b

  check_endian();
  show_mem_rep((char*)&cid.volumeid, sizeof(cid.volumeid)); // 65 00 00 00
  printf(" |");
  show_mem_rep((char*)&cid.index, sizeof(cid.index));       // 7b 00 00 00
  printf("\n");

  printf("         bits: (%llu)\n", cid.bits);        // 528280977509
  printf("  PRIu64 bits: (%" PRIu64 ")\n", cid.bits); // 528280977509
  printf("  bits in hex: (%llx)\n", cid.bits);        // 7b00000065
  printf("bits hex prex: (%016llx)\n", cid.bits);     // 0000007b00000065
  show_mem_rep((char*)&cid.bits, sizeof(cid.bits));   // 65 00 00 00 7b 00 00 00
  printf("\n");
  // it should be related to 32bit / 64bit system
  // 64位系统，自然值=后32bit小端转大端 接 先32bit小端转大端
  printf("0x650000007b000000=%lu\n", 0x650000007b000000); // 7277816999894319104
  printf("0x000000650000007b=%lu\n", 0x000000650000007b); // 433791697019
  printf("0x7b00000065=%lu\n", 0x7b00000065);             // 528280977509
  printf("0x0000007b00000065=%lu\n", 0x0000007b00000065); // 528280977509
}
