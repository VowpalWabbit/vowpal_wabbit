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

hash_func_t getHasher(const std::string& s)
{
  if (s == "strings")
    return hashstring;
  else if (s == "all")
    return hashall;
  else
    THROW("Unknown hash function: " << s);
}

std::vector<std::string> escaped_tokenize(char delim, VW::string_view s, bool allow_empty)
{
  std::vector<std::string> tokens;
  std::string current;
  size_t end_pos = 0;
  const char delims[3] = {'\\', delim, '\0'};
  bool last_space = false;

  while (!s.empty() && ((end_pos = s.find_first_of(delims)) != VW::string_view::npos))
  {
    if(s[end_pos] == '\\')
    {
      current.append(s.begin(), end_pos);
      s.remove_prefix(end_pos + 1);

      // always insert the next character after an escape if it exists
      if(!s.empty())
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
      if(!current.empty() || allow_empty)
      {
        tokens.push_back(current);
      }
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

std::ostream& operator<<(std::ostream& os, const v_array<VW::string_view>& ss)
{
  VW::string_view* it = ss.cbegin();

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
