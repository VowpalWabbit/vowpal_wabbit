// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cctype>

#include "parse_primitives.h"
#include "hash.h"
#include "vw_exception.h"
#include "vw_string_view.h"

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
  std::string current_str;
  for (It current = begin; current != end; ++current)
  {
    if (is_escape_char(*current))
    {
      current++;
      current_str.append(1, unescape_char(current, end));
    }
    else if (is_delim(*current))
    {
      // If we're not inside a token, this token is done. Otherwise just add the space to the token.
      if (!inside_quote)
      {
        if (!current_str.empty()) { ret.push_back(current_str); }
        current_str.clear();
      }
      else
      {
        current_str.append(1, *current);
      }
    }
    else if (is_quote(*current))
    {
      if (inside_quote && quote_char == *current) { inside_quote = false; }
      else if (!inside_quote)
      {
        inside_quote = true;
        quote_char = *current;
      }
      else
      {
        current_str.append(1, *current);
      }
    }
    else
    {
      current_str.append(1, *current);
    }
  }

  if (inside_quote) {
    std::string input_string{begin, end};
    THROW("unbalanced quotes in string: " << input_string); }

  if (!current_str.empty()) { ret.push_back(current_str); }
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

std::string escape_string(VW::string_view str)
{
  std::stringstream ss;
  for (char c : str)
  {
    switch (c)
    {
      case '\n':
        ss << R"(\n)";
        break;
      case '\t':
        ss << R"(\t)";
        break;
      case '\\':
        ss << R"(\)";
        break;
      case '"':
        ss << R"(\")";
        break;
      case '\'':
        ss << R"(\')";
        break;
      default:
        ss << c;
        break;
    }
  }
  return ss.str();
}

bool contains_escapeable_chars(VW::string_view str)
{
  const std::set<char> escape_chars = {'\n', '\t', '\\', '"', '\''};
  for (auto c : str)
  {
    if (escape_chars.find(c) != escape_chars.end()) { return true; }
  }
  return false;
}


}  // namespace VW
