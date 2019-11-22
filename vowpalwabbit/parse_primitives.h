/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <math.h>
#include "v_array.h"
#include "hashstring.h"

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#include <Windows.h>
#endif

std::ostream& operator<<(std::ostream& os, const substring& ss);
std::ostream& operator<<(std::ostream& os, const v_array<substring>& ss);

// chop up the  string into a v_array or any compatible container of substring.
template <typename ContainerT>
void tokenize(char delim, substring s, ContainerT& ret, bool allow_empty = false)
{
  ret.clear();
  char* last = s.begin;
  for (; s.begin != s.end; s.begin++)
  {
    if (*s.begin == delim)
    {
      if (allow_empty || (s.begin != last))
      {
        substring temp = {last, s.begin};
        ret.push_back(temp);
      }
      last = s.begin + 1;
    }
  }
  if (allow_empty || (s.begin != last))
  {
    substring final_substring = {last, s.begin};
    ret.push_back(final_substring);
  }
}

bool substring_equal(const substring& a, const substring& b);
bool substring_equal(const substring& ss, const char* str);

bool operator==(const substring& ss, const char* str);
bool operator==(const char* str, const substring& ss);
bool operator==(const substring& ss1, const substring& ss2);
bool operator!=(const substring& ss, const char* str);
bool operator!=(const char* str, const substring& ss);
bool operator!=(const substring& ss1, const substring& ss2);
size_t substring_len(substring& s);

inline char* safe_index(char* start, char v, char* max)
{
  while (start != max && *start != v) start++;
  return start;
}

// Note this will destructively parse the passed in substring as it replaces delimiters with '\0'
std::vector<substring> escaped_tokenize(char delim, substring s, bool allow_empty = false);

inline void print_substring(substring s) { std::cout.write(s.begin, s.end - s.begin); }

// can't type as it forces C++/CLI part to include rapidjson, which leads to name clashes...
struct example;
namespace VW
{
typedef example& (*example_factory_t)(void*);
}

typedef uint64_t (*hash_func_t)(substring, uint64_t);

hash_func_t getHasher(const std::string& s);

// The following function is a home made strtof. The
// differences are :
//  - much faster (around 50% but depends on the  string to parse)
//  - less error control, but utilised inside a very strict parser
//    in charge of error detection.
inline float parseFloat(char* p, char** end, char* endLine = nullptr)
{
  char* start = p;
  bool endLine_is_null = endLine == nullptr;

  if (!*p)
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
    return (float)strtod(start, end);
}

inline float float_of_substring(substring s)
{
  char* endptr = s.end;
  float f = parseFloat(s.begin, &endptr);
  if ((endptr == s.begin && s.begin != s.end) || std::isnan(f))
  {
    std::cout << "warning: " << std::string(s.begin, s.end - s.begin).c_str()
              << " is not a good float, replacing with 0" << std::endl;
    f = 0;
  }
  return f;
}

inline int int_of_substring(substring s)
{
  char* endptr = s.end;
  int i = strtol(s.begin, &endptr, 10);
  if (endptr == s.begin && s.begin != s.end)
  {
    std::cout << "warning: " << std::string(s.begin, s.end - s.begin).c_str() << " is not a good int, replacing with 0"
              << std::endl;
    i = 0;
  }

  return i;
}
