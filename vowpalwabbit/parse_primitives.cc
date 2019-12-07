/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <iostream>
#ifndef WIN32
#include <strings.h>
#else
#include <string>
#endif
#include <stdexcept>
#include <sstream>

#include "parse_primitives.h"
#include "hash.h"
#include "vw_exception.h"

bool substring_equal(const substring& a, const substring& b)
{
  return (a.end - a.begin == b.end - b.begin)  // same length
      && (strncmp(a.begin, b.begin, a.end - a.begin) == 0);
}

bool substring_equal(const substring& ss, const char* str)
{
  size_t len_ss = ss.end - ss.begin;
  size_t len_str = strlen(str);
  if (len_ss != len_str)
    return false;
  return (strncmp(ss.begin, str, len_ss) == 0);
}

size_t substring_len(substring& s) { return s.end - s.begin; }

hash_func_t getHasher(const std::string& s)
{
  if (s == "strings")
    return hashstring;
  else if (s == "all")
    return hashall;
  else
    THROW("Unknown hash function: " << s);
}

bool operator==(const substring& ss, const char* str) { return substring_equal(ss, str); }

bool operator==(const char* str, const substring& ss) { return substring_equal(ss, str); }

bool operator==(const substring& ss1, const substring& ss2) { return substring_equal(ss1, ss2); }

bool operator!=(const substring& ss, const char* str) { return !(ss == str); }

bool operator!=(const char* str, const substring& ss) { return !(ss == str); }

bool operator!=(const substring& ss1, const substring& ss2) { return !(ss1 == ss2); }

std::vector<substring> escaped_tokenize(char delim, substring s, bool allow_empty)
{
  std::vector<substring> tokens;
  substring current;
  current.begin = s.begin;
  bool in_escape = false;
  char* reading_head = s.begin;
  char* writing_head = s.begin;

  while (reading_head < s.end)
  {
    char current_character = *reading_head++;

    if (in_escape)
    {
      *writing_head++ = current_character;
      in_escape = false;
    }
    else
    {
      if (current_character == delim)
      {
        current.end = writing_head++;
        *current.end = '\0';
        if (current.begin != current.end || allow_empty)
        {
          tokens.push_back(current);
        }

        // Regardless of whether the token was saved, we need to reset the current token.
        current.begin = writing_head;
        current.end = writing_head;
      }
      else if (current_character == '\\')
      {
        in_escape = !in_escape;
      }
      else
      {
        *writing_head++ = current_character;
      }
    }
  }

  current.end = writing_head;
  *current.end = '\0';
  if (current.begin != current.end || allow_empty)
  {
    tokens.push_back(current);
  }

  return tokens;
}

std::ostream& operator<<(std::ostream& os, const substring& ss)
{
  std::string s(ss.begin, ss.end - ss.begin);
  return os << s;
}

std::ostream& operator<<(std::ostream& os, const v_array<substring>& ss)
{
  substring* it = ss.cbegin();

  if (it == ss.cend())
  {
    return os;
  }

  os << *it;

  for (it++; it != ss.cend(); it++)
  {
    os << ",";
    os << *it;
  }

  return os;
}
