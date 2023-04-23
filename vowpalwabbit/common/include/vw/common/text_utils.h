// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "string_view.h"

#include <sstream>
#include <vector>

namespace VW
{
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
    if (allow_empty || end_pos > 0) { ret.emplace_back(s.substr(0, end_pos)); }
    s.remove_prefix(end_pos + 1);
  }
  if (!s.empty() || (last_space && allow_empty)) { ret.emplace_back(s.substr(0)); }
}

/**
 * \brief Check if a string ends with some other string.
 * \param full_string String to check ending of
 * \param ending Ending value to check
 * \return true if full_string ends with ending, otherwise false.
 */
inline bool ends_with(VW::string_view full_string, VW::string_view ending)
{
  return full_string.size() >= ending.size() &&
      0 == full_string.compare(full_string.size() - ending.size(), ending.size(), ending);
}

/**
 * \brief Check if a string starts with some other string.
 * \param full_string String to check starting of
 * \param starting Starting value to check
 * \return true if full_string starts with starting, otherwise false.
 */
inline bool starts_with(VW::string_view full_string, VW::string_view starting)
{
  return full_string.size() >= starting.size() && 0 == full_string.compare(0, starting.size(), starting);
}

/**
 * @brief Wrap text by whole words with the given column width.
 *
 * @param text text to wrap
 * @param width column width to wrap to
 * @param wrap_after if word causes line to exceed width include word on same line. If false, this word would be wrapped
 * to the next line.
 * @return std::string copy of string with required newlines
 */
inline std::string wrap_text(VW::string_view text, size_t width, bool wrap_after = true)
{
  std::stringstream ss;
  std::vector<VW::string_view> words;
  VW::tokenize(' ', text, words);
  size_t current_line_size = 0;
  std::string space = "";
  for (const auto& word : words)
  {
    if ((wrap_after && current_line_size > width) || (!wrap_after && (current_line_size + word.size() > width)))
    {
      ss << '\n';
      space = "";
      current_line_size = 0;
    }
    ss << space << word;
    space = " ";
    current_line_size += word.size() + 1;
  }
  return ss.str();
}
}  // namespace VW