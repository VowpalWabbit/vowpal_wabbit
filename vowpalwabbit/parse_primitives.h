/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <math.h>
#include "v_array.h"
#include "hashstring.h"
#include <boost/utility/string_view.hpp>

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif

std::ostream& operator<<(std::ostream& os, const v_array<boost::string_view>& ss);

// chop up the string into a v_array or any compatible container of string_view.
template <typename ContainerT>
void tokenize(char delim, const boost::string_view s, ContainerT& ret, bool allow_empty = false)
{
  ret.clear();
  size_t start_pos = 0;
  size_t end_pos = 0;

  while ((end_pos = s.find(delim, start_pos)) != boost::string_view::npos)
  {
    if (allow_empty || start_pos != end_pos)
      ret.emplace_back(s.substr(start_pos, end_pos - start_pos));
    start_pos = end_pos + 1;
  }
  if (start_pos < s.size())
    ret.emplace_back(s.substr(start_pos));
}

inline const char* safe_index(const char* start, char v, const char* max)
{
  while (start != max && *start != v) start++;
  return start;
}

// can't type as it forces C++/CLI part to include rapidjson, which leads to name clashes...
struct example;
namespace VW
{
typedef example& (*example_factory_t)(void*);
}

typedef uint64_t (*hash_func_t)(boost::string_view, uint64_t);

hash_func_t getHasher(const std::string& s);

// The following function is a home made strtof. The
// differences are :
//  - much faster (around 50% but depends on the string to parse)
//  - less error control, but utilised inside a very strict parser
//    in charge of error detection.
inline float parseFloat(const char* p, const char** end, const char* endLine = nullptr)
{
  const char* start = p;
  bool endLine_is_null = endLine == nullptr;

  if (!p || !*p)
  {
    *end = p;
    return 0;
  }
  int s = 1;
  while ((*p == ' ') && (endLine_is_null || p < endLine)) p++;

  if (*p == '-')
  {
    s = -1;
    p++;
  }

  float acc = 0;
  while (*p >= '0' && *p <= '9' && (endLine_is_null || p < endLine)) acc = acc * 10 + *p++ - '0';

  int num_dec = 0;
  if (*p == '.')
  {
    while (*(++p) >= '0' && *p <= '9' && (endLine_is_null || p < endLine))
    {
      if (num_dec < 35)
      {
        acc = acc * 10 + (*p - '0');
        num_dec++;
      }
    }
  }

  int exp_acc = 0;
  if ((*p == 'e' || *p == 'E') && (endLine_is_null || p < endLine))
  {
    p++;
    int exp_s = 1;
    if (*p == '-' && (endLine_is_null || p < endLine))
    {
      exp_s = -1;
      p++;
    }
    while (*p >= '0' && *p <= '9' && (endLine_is_null || p < endLine)) exp_acc = exp_acc * 10 + *p++ - '0';
    exp_acc *= exp_s;
  }
  if (*p == ' ' || *p == '\n' || *p == '\t' || p == endLine)  // easy case succeeded.
  {
    acc *= powf(10, (float)(exp_acc - num_dec));
    *end = p;
    return s * acc;
  }
  else
    // const_cast is bad, but strtod requires end to be a non-const char**
    return (float)strtod(start, const_cast<char**>(end));
}

inline float parse_float_string_view(boost::string_view strview, size_t& end_idx)
{
  const char* end = nullptr;
  float ret = parseFloat(strview.begin(), &end, strview.end());
  end_idx = std::distance(strview.begin(), end);
  return ret;
}

inline float float_of_string(boost::string_view s)
{
  size_t end_idx;
  float f = parse_float_string_view(s, end_idx);
  if ((end_idx == 0 && s.size() > 0) || std::isnan(f))
  {
    std::cout << "warning: " << s << " is not a good float, replacing with 0" << std::endl;
    f = 0;
  }
  return f;
}

inline int int_of_string(boost::string_view s)
{
  const char* endptr = s.end();
  int i = strtol(s.begin(), const_cast<char**>(&endptr), 10);
  if (endptr == s.begin() && s.size() > 0)
  {
    std::cout << "warning: " << s << " is not a good int, replacing with 0"
              << std::endl;
    i = 0;
  }

  return i;
}
