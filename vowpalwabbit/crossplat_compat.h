// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include <cstring>
#include <string.h>
#include <stdio.h>
#include <cstdio>

#ifndef _WIN32
#  define sprintf_s snprintf
#  define vsprintf_s vsnprintf
#  define strtok_s strtok_r
#  define fscanf_s fscanf

constexpr uint64_t UINT64_ZERO = 0ULL;
constexpr uint64_t UINT64_ONE = 1ULL;
#else
constexpr uint64_t UINT64_ONE = 1i64;
constexpr uint64_t UINT64_32ONES = 0x00000000ffffffffi64;
#endif

namespace VW
{
inline int string_cpy(char* dest, size_t dest_size, const char* src)
{
#ifdef _WIN32
  // strcpy_s returns an errno_t
  return strcpy_s(dest, dest_size, src);
#else
  (void)dest_size;  // unused here
  strcpy(dest, src);
  return 0;
#endif
}

inline int file_open(FILE** pf, const char* filename, const char* mode)
{
#ifdef _WIN32
  // fopen_s returns an errno_t
  return fopen_s(pf, filename, mode);
#else
  *pf = fopen(filename, mode);
  if (*pf == nullptr) { return -1; }
  return 0;
#endif
}
}  // namespace VW
