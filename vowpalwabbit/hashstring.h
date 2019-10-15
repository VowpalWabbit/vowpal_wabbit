#pragma once
#include <cstdint>  // defines size_t
#include <boost/utility/string_view.hpp>
#include "hash.h"

#include "future_compat.h"

VW_STD14_CONSTEXPR inline uint64_t hashall(boost::string_view s, uint64_t h)
{
  return uniform_hash((unsigned char*)s.begin(), s.size(), h);
}

VW_STD14_CONSTEXPR inline uint64_t hashstring(boost::string_view s, uint64_t h)
{
  // trim leading whitespace but not UTF-8
  while (!s.empty() && s.front() <= 0x20 && (int)(s.front()) >= 0) s.remove_prefix(1);
  // trim trailing white space but not UTF-8
  while (!s.empty() && s.back() <= 0x20 && (int)(s.back()) >= 0) s.remove_suffix(1);

  size_t ret = 0;
  const char* p = s.begin();
  while (p != s.end())
    if (*p >= '0' && *p <= '9')
      ret = 10 * ret + *(p++) - '0';
    else
      return uniform_hash((unsigned char*)s.begin(), s.size(), h);

  return ret + h;
}

namespace std
{
// boost string_view hashing isn't available until 1.69. Implement our own for now
template <>
struct hash<boost::string_view>
{
  size_t operator()(const boost::string_view& s) const { return hashstring(s, 0); }
};
}  // namespace std
