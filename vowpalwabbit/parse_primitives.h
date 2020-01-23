// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cmath>
#include "v_array.h"
#include "hashstring.h"
#include "vw_string_view.h"

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <Windows.h>
#endif

std::ostream& operator<<(std::ostream& os, const v_array<VW::string_view>& ss);

// chop up the string into a v_array or any compatible container of VW::string_view.
template <typename ContainerT>
void tokenize(char delim, VW::string_view s, ContainerT& ret, bool allow_empty = false)
{
  ret.clear();
  size_t end_pos = 0;
  bool last_space = false;

  while (!s.empty() && ((end_pos = s.find(delim)) != VW::string_view::npos))
  {
    last_space = end_pos == 0;
    if (allow_empty || end_pos > 0)
      ret.emplace_back(s.substr(0, end_pos));
    s.remove_prefix(end_pos + 1);
  }
  if (!s.empty() || (last_space && allow_empty))
    ret.emplace_back(s.substr(0));
}

// This function returns a vector of strings (not string_views) because we need to remove the escape characters
std::vector<std::string> escaped_tokenize(char delim, VW::string_view s, bool allow_empty = false);

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

typedef uint64_t (*hash_func_t)(const char * s, size_t, uint64_t);

hash_func_t getHasher(const std::string& s);

// The following function is a home made strtof. The
// differences are :
//  - much faster (around 50% but depends on the  string to parse)
//  - less error control, but utilised inside a very strict parser
//    in charge of error detection.
inline float parseFloat(const char* p, size_t& end_idx, const char* endLine = nullptr)
{
  const char* start = p;
  bool endLine_is_null = endLine == nullptr;

  end_idx = 0;

  if (!p || !*p)
  {
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
    end_idx = p - start;
    return s * acc;
  }
  else
  {
    // can't use stod because that throws an exception. Use strtod instead.
    char* end = nullptr;
    auto ret = strtod(start, &end);
    if (end >= start)
    {
      end_idx = end - start;
    }
    return ret;
  }
    
}

inline float float_of_string(VW::string_view s)
{
  size_t end_idx;
  float f = parseFloat(s.begin(), end_idx, s.end());
  if ((end_idx == 0 && s.size() > 0) || std::isnan(f))
  {
    std::cout << "warning: " << s << " is not a good float, replacing with 0" << std::endl;
    f = 0;
  }
  return f;
}

inline int int_of_string(VW::string_view s)
{
  char* end = nullptr;

  // can't use stol because that throws an exception. Use strtol instead.
  int i = strtol(s.begin(), &end, 10);
  if (end <= s.begin() && s.size() > 0)
  {
    std::cout << "warning: " << s << " is not a good int, replacing with 0"
              << std::endl;
    i = 0;
  }

  return i;
}
