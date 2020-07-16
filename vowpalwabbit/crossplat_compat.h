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
#define sprintf_s snprintf
#define vsprintf_s vsnprintf
#define strtok_s strtok_r
#define fscanf_s fscanf

constexpr uint64_t UINT64_ZERO = 0ULL;
constexpr uint64_t UINT64_ONE = 1ULL;
#else
constexpr uint64_t UINT64_ONE = 1i64;
constexpr uint64_t UINT64_32ONES = 0x00000000ffffffffi64;
#endif

namespace VW
{
inline void string_cpy(char *dest, size_t dest_size, const char *src, int *error_no = nullptr)
{
#ifdef _WIN32
  // strcpy_s returns an errno_t
  auto err = strcpy_s(dest, dest_size, src);
  if (error_no) { *error_no = err; }
#else
  strncpy(dest, src, dest_size);
  if (error_no) { *error_no = 0; }
#endif
}

inline void fopen_(FILE **pf, const char *filename, const char *mode, int *error_no = nullptr)
{
#ifdef _WIN32
  // fopen_s returns an errno_t
  auto err = fopen_s(pf, filename, mode);
  if (error_no) { *error_no = err; }
#else
  *pf = fopen(filename, mode);
  if (error_no)
  {
    if (*pf != nullptr) { *error_no = 0; }
    else
    {
      *error_no = 1;
    }
  }
#endif
}
}  // namespace VW
