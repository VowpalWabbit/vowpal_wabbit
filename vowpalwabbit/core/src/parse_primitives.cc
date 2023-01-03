// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parse_primitives.h"

#include "vw/common/hash.h"
#include "vw/common/string_view.h"
#include "vw/common/vw_exception.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

std::vector<std::string> VW::details::escaped_tokenize(char delim, VW::string_view s, bool allow_empty)
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
      current.append(s.data(), end_pos);
      s.remove_prefix(end_pos + 1);

      // always insert the next character after an escape if it exists
      if (!s.empty())
      {
        current.append(s.data(), 1);
        s.remove_prefix(1);
      }
    }
    else
    {
      last_space = end_pos == 0;
      current.append(s.data(), end_pos);
      s.remove_prefix(end_pos + 1);
      if (!current.empty() || allow_empty) { tokens.push_back(current); }
      current.clear();
    }
  }
  // write whatever's left into the vector
  if (!s.empty() || !current.empty() || (last_space && allow_empty))
  {
    current.append(s.data(), s.length());
    tokens.push_back(current);
  }
  return tokens;
}

bool is_delim(char c) { return c == ' '; }

bool is_quote(char c) { return c == '"' || c == '\''; }

bool is_escape_char(char c) { return c == '\\'; }

template <typename It>
char unescape_char(It char_to_unescape_it, It end)
{
  if (char_to_unescape_it == end) { THROW("unescape_char: unexpected end of string while unescaping"); }
  char c = *char_to_unescape_it;
  if (c == 'n') { return '\n'; }
  if (c == 't') { return '\t'; }
  return c;
}

template <typename It>
std::vector<std::string> split_impl(It begin, It end)
{
  std::vector<std::string> ret;
  if (begin == end) { return ret; }

  bool inside_quote = false;
  char quote_char = '\0';
  std::string current;
  for (; begin != end; ++begin)
  {
    if (is_escape_char(*begin))
    {
      begin++;
      current.append(1, unescape_char(begin, end));
    }
    else if (is_delim(*begin))
    {
      // If we're not inside a token, this token is done. Otherwise just add the space to the token.
      if (!inside_quote)
      {
        if (!current.empty()) { ret.push_back(current); }
        current.clear();
      }
      else { current.append(1, *begin); }
    }
    else if (is_quote(*begin))
    {
      if (inside_quote && quote_char == *begin) { inside_quote = false; }
      else if (!inside_quote)
      {
        inside_quote = true;
        quote_char = *begin;
      }
      else { current.append(1, *begin); }
    }
    else { current.append(1, *begin); }
  }

  if (inside_quote) { THROW("unbalanced quotes in string"); }

  if (!current.empty()) { ret.push_back(current); }
  return ret;
}

namespace VW
{
std::string trim_whitespace(const std::string& str) { return std::string(VW::trim_whitespace(VW::string_view(str))); }

VW::string_view trim_whitespace(VW::string_view str)
{
  // Determine start
  auto start = std::find_if_not(str.begin(), str.end(), [](char c) { return std::isspace(c); });
  if (start == str.end()) { return ""; }
  auto start_pos = std::distance(str.begin(), start);

  // Determine end
  auto end = std::find_if_not(str.rbegin(), str.rend(), [](char c) { return std::isspace(c); });
  if (end == str.rend()) { return ""; }
  // -1 is required as position 0 of the string is (rend - 1)
  auto end_pos = std::distance(end, str.rend()) - 1;
  return str.substr(start_pos, (end_pos - start_pos) + 1);
}

std::vector<std::string> split_command_line(VW::string_view cmd_line)
{
  return split_impl(cmd_line.begin(), cmd_line.end());
}

std::vector<std::string> split_command_line(const std::string& cmd_line)
{
  return split_impl(cmd_line.begin(), cmd_line.end());
}

std::vector<VW::string_view> split_by_limit(const VW::string_view& s, size_t limit)
{
  std::vector<VW::string_view> result;
  size_t start = 0;
  while (start < s.size())
  {
    size_t end = start + limit;
    if (end > s.size()) { end = s.size(); }
    result.push_back(s.substr(start, end - start));
    start = end;
  }
  return result;
}
}  // namespace VW
