// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifndef _WIN32
#  define sprintf_s snprintf
#  define vsprintf_s vsnprintf
#  define strtok_s strtok_r
#  define fscanf_s fscanf
#endif

namespace VW
{
namespace details
{
#ifndef _WIN32
constexpr uint64_t UINT64_ONE = 1ULL;
#else
constexpr uint64_t UINT64_ONE = 1i64;
#endif

}  // namespace details
int string_cpy(char* dest, size_t dest_size, const char* src);
int file_open(FILE** pf, const char* filename, const char* mode);
int get_pid();
}  // namespace VW
