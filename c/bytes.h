#ifndef __BYTES_H__
#define __BYTES_H__

#include <stdint.h>
#include <stdbool.h>

static inline bool is_big_endian()
{
  int v = 0x0D0C0B0A;
  char c = *(char*) &v;
  if (c == 0x0A) return false;
  return true;
}

static inline uint16_t byte_swap2(uint16_t original)
{
  uint16_t swapped;
  swapped = ((original & 0xff00) >> 8) | ((original & 0x00ff) << 8);
  return swapped;
}

static inline uint32_t byte_swap4(uint32_t original)
{
  uint32_t swapped;

  // ABCD => BADC
  swapped = ((original & 0xff00ff00) >> 8) | ((original & 0x00ff00ff) << 8);

  // BADC => DCBA
  swapped = ((swapped & 0xffff0000) >> 16) | ((swapped & 0x0000ffff) << 16);

  return swapped; 
}

static inline uint64_t byte_swap8( uint64_t original )
{
  uint64_t swapped;

  // ABCDEFGH => BADCFEHG
  swapped = ((original & 0xff00ff00ff00ff00ULL) >> 8) | ((original & 0x00ff00ff00ff00ffULL) << 8);

  // BADCFEHG => DCBAHGFE
  swapped = ((swapped & 0xffff0000ffff0000ULL) >> 16) | ((swapped & 0x0000ffff0000ffffULL) << 16);

  // DCBAHGFE => HGFEDCBA
  swapped = ((swapped & 0xffffffff00000000ULL) >> 32) | ((swapped & 0x00000000ffffffffULL) << 32);

  return swapped;
}

#endif
