#pragma once
#include <cstdint>  // defines size_t
#include "hash.h"

#include "future_compat.h"

struct substring
{
  char* begin;
  char* end;
};

VW_STD14_CONSTEXPR inline uint64_t hashall(substring s, uint64_t h)
{
  return uniform_hash((unsigned char*)s.begin, s.end - s.begin, h);
}

VW_STD14_CONSTEXPR inline uint64_t hashstring(substring s, uint64_t h)
{
  // trim leading whitespace but not UTF-8
  for (; s.begin < s.end && *(s.begin) <= 0x20 && (int)*(s.begin) >= 0; s.begin++)
    ;
  // trim trailing white space but not UTF-8
  for (; s.end > s.begin && *(s.end - 1) <= 0x20 && (int)*(s.end - 1) >= 0; s.end--)
    ;

  size_t ret = 0;
  char* p = s.begin;
  while (p != s.end)
    if (*p >= '0' && *p <= '9')
      ret = 10 * ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char*)s.begin, s.end - s.begin, h);

  return ret + h;
}
