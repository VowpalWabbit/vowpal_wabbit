// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include "parse_primitives.h"
#include "hash.h"
#include "vw_exception.h"

std::vector<std::string> escaped_tokenize(char delim, VW::string_view s, bool allow_empty)
{
  std::vector<std::string> tokens;
  std::string current;
  size_t end_pos = 0;
  const char delims[3] = {'\\', delim, '\0'};
  bool last_space = false;

  while (!s.empty() && ((end_pos = s.find_first_of(delims)) != VW::string_view::npos))
  {
    if (s[end_pos] == '\\')
    {
      current.append(s.begin(), end_pos);
      s.remove_prefix(end_pos + 1);

      // always insert the next character after an escape if it exists
      if (!s.empty())
      {
        current.append(s.begin(), 1);
        s.remove_prefix(1);
      }
    }
    else
    {
      last_space = end_pos == 0;
      current.append(s.begin(), end_pos);
      s.remove_prefix(end_pos + 1);
      if (!current.empty() || allow_empty) { tokens.push_back(current); }
      current.clear();
    }
  }
  // write whatever's left into the vector
  if (!s.empty() || !current.empty() || (last_space && allow_empty))
  {
    current.append(s.begin(), s.length());
    tokens.push_back(current);
  }
  return tokens;
}

constexpr const char* whitespace_chars_to_remove = " \n\r\t";
namespace VW
{
std::string trim_whitespace(const std::string& str)
{
  // Determine start
  size_t start = str.find_first_not_of(whitespace_chars_to_remove);
  if (start == std::string::npos) { return ""; }

  // Determine end
  size_t end = str.find_last_not_of(whitespace_chars_to_remove);
  if (end == std::string::npos) { return ""; }
  return str.substr(start, end + 1);
}

VW::string_view trim_whitespace(VW::string_view str)
{
  // Determine start
  size_t start = str.find_first_not_of(whitespace_chars_to_remove);
  if (start == std::string::npos) { return ""; }
  str = str.substr(start);

  // Determine end
  size_t end = str.find_last_not_of(whitespace_chars_to_remove);
  if (end == std::string::npos) { return ""; }
  return str.substr(0, end + 1);
}
}  // namespace VW