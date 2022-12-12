// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/text_utils.h"
#include "vw/core/parse_primitives.h"

#include <array>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace VW
{
enum class wrap_type
{
  truncate,
  truncate_with_ellipsis,
  wrap_space,
  wrap_char
};

enum class align_type
{
  left,
  right
};

class column_definition
{
public:
  size_t column_width;
  align_type alignment;
  wrap_type wrapping;

  constexpr column_definition(size_t column_width, align_type alignment, wrap_type wrapping)
      : column_width(column_width), alignment(alignment), wrapping(wrapping)
  {
  }
};

template <size_t num_cols>
void format_row(const std::array<std::string, num_cols>& contents,
    const std::array<column_definition, num_cols>& column_definitions, size_t column_padding, std::ostream& output)
{
  std::streamsize saved_w = output.width();
  std::ostream::fmtflags saved_f = output.flags();
  std::array<std::vector<std::string>, num_cols> column_contents_split_into_lines;
  // Wrap contents into lines
  for (size_t i = 0; i < num_cols; i++)
  {
    if (column_definitions[i].wrapping == wrap_type::wrap_space)
    {
      VW::tokenize('\n', VW::wrap_text(contents[i], column_definitions[i].column_width, false),
          column_contents_split_into_lines[i]);
    }
    else if (column_definitions[i].wrapping == wrap_type::wrap_char)
    {
      for (const auto& token : split_by_limit(contents[i], column_definitions[i].column_width))
      {
        column_contents_split_into_lines[i].push_back(std::string{token});
      }
    }
    else { column_contents_split_into_lines[i].push_back(contents[i]); }

    for (auto& line : column_contents_split_into_lines[i])
    {
      if (column_definitions[i].wrapping == wrap_type::truncate_with_ellipsis &&
          line.size() > column_definitions[i].column_width)
      {
        line = line.substr(0, column_definitions[i].column_width - 3) + "...";
      }
      else { line = line.substr(0, column_definitions[i].column_width); }
    }
  }

  // Find the maximum number of lines in each column
  size_t max_num_lines = 0;
  for (size_t i = 0; i < num_cols; i++)
  {
    max_num_lines = std::max(max_num_lines, column_contents_split_into_lines[i].size());
  }

  // The final newline is NOT printed.
  std::string delim = "";
  std::string padding = "";
  for (size_t line = 0; line < max_num_lines; line++)
  {
    output << delim;
    delim = "\n";
    for (size_t col = 0; col < num_cols; col++)
    {
      for (size_t i = 0; i < column_padding; i++) { output << padding; }
      padding = " ";
      if (line < column_contents_split_into_lines[col].size())
      {
        if (column_definitions[col].alignment == align_type::left)
        {
          output << std::left << std::setw(column_definitions[col].column_width)
                 << column_contents_split_into_lines[col][line];
        }
        else
        {
          output << std::right << std::setw(column_definitions[col].column_width)
                 << column_contents_split_into_lines[col][line];
        }
      }
      else { output << std::setw(column_definitions[col].column_width) << ""; }
    }
    padding = "";
  }

  output.width(saved_w);
  output.setf(saved_f);
}

template <size_t num_cols>
std::string format_row(const std::array<std::string, num_cols>& contents,
    const std::array<column_definition, num_cols>& column_definitions, size_t column_padding)
{
  std::ostringstream ss;
  format_row(contents, column_definitions, column_padding, ss);
  return ss.str();
}

}  // namespace VW
