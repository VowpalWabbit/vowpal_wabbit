// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>

#ifndef _WIN32
#define sprintf_s snprintf
#define vsprintf_s vsnprintf

constexpr uint64_t UINT64_ZERO = 0ULL;
constexpr uint64_t UINT64_ONE = 1ULL;
#else
constexpr uint64_t UINT64_ONE = 1i64;
constexpr uint64_t UINT64_32ONES = 0x00000000ffffffffi64;
#endif
