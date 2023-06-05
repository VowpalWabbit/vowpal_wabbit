// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/common/future_compat.h"
#include "vw/common/hash.h"

#include <cstddef>  // defines size_t
#include <cstdint>
#include <string>

namespace VW
{
namespace details
{
VW_STD14_CONSTEXPR inline uint32_t hashall(const char* s, size_t len, uint32_t h)
{
  return VW::uniform_hash(s, len, h);
}

VW_STD14_CONSTEXPR inline uint32_t hashstring(const char* s, size_t len, uint32_t h)
{
  const char* front = s;
  while (len > 0 && front[0] <= 0x20 && static_cast<int>(front[0]) >= 0)
  {
    ++front;
    --len;
  }
  while (len > 0 && front[len - 1] <= 0x20 && static_cast<int>(front[len - 1]) >= 0) { --len; }

  size_t ret = 0;
  const char* p = front;
  while (p != front + len)
  {
    if (*p >= '0' && *p <= '9') { ret = 10 * ret + *(p++) - '0'; }
    else { return VW::uniform_hash(front, len, h); }
  }

  return ret + h;
}
}  // namespace details

// hash_func_t is always either hashall or hashstring as defined above
// so we will use a raw function pointer here instead of std::function
using hash_func_t = uint32_t (*)(const char*, size_t, uint32_t);

hash_func_t get_hasher(const std::string& s);

}  // namespace VW
using hash_func_t VW_DEPRECATED("Moved into VW namespace") = VW::hash_func_t;

VW_DEPRECATED("Moved into VW namespace")
VW::hash_func_t get_hasher(const std::string& s);