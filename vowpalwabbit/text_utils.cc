// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "text_utils.h"

#include "io/logger.h"
#include "parse_primitives.h"

#include <fmt/format.h>

#include <sstream>

namespace VW
{
bool ends_with(VW::string_view full_string, VW::string_view ending)
{
  return full_string.size() >= ending.size() &&
      0 == full_string.compare(full_string.size() - ending.size(), ending.size(), ending);
}

bool starts_with(VW::string_view full_string, VW::string_view starting)
{
  return full_string.size() >= starting.size() && 0 == full_string.compare(0, starting.size(), starting);
}

std::string decode_inline_hex(VW::string_view arg, VW::io::logger& logger)
{
  constexpr size_t NUMBER_OF_HEX_CHARS = 2;
  // "\x" + hex chars
  constexpr size_t LENGTH_OF_HEX_TOKEN = 2 + NUMBER_OF_HEX_CHARS;
  constexpr size_t HEX_BASE = 16;

  // Too short to be hex encoded.
  if (arg.size() < LENGTH_OF_HEX_TOKEN) { return std::string{arg}; }

  std::string res;
  size_t pos = 0;
  while (pos < arg.size() - (LENGTH_OF_HEX_TOKEN - 1))
  {
    if (arg[pos] == '\\' && arg[pos + 1] == 'x')
    {
      auto substr = std::string{arg.substr(pos + NUMBER_OF_HEX_CHARS, NUMBER_OF_HEX_CHARS)};
      char* p;
      const auto c = static_cast<char>(std::strtoul(substr.c_str(), &p, HEX_BASE));
      if (*p == '\0')
      {
        res.push_back(c);
        pos += LENGTH_OF_HEX_TOKEN;
      }
      else
      {
        logger.err_warn("Possibly malformed hex representation of a namespace: '\\x{}'", substr);
        res.push_back(arg[pos++]);
      }
    }
    else
    {
      res.push_back(arg[pos++]);
    }
  }

  // Copy last 2 characters
  while (pos < arg.size()) { res.push_back(arg[pos++]); }

  return res;
}

std::string wrap_text(VW::string_view text, size_t width, bool wrap_after)
{
  std::stringstream ss;
  std::vector<VW::string_view> words;
  tokenize(' ', text, words);
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

// max_decimal_places < 0 means use as many decimal places as necessary
std::string fmt_float(float f, int max_decimal_places)
{
  if (max_decimal_places >= 0)
  {
    auto formatted = fmt::format("{:.{}f}", f, max_decimal_places);
    while (formatted.back() == '0') { formatted.pop_back(); }
    if (formatted.back() == '.') { formatted.pop_back(); }

    return formatted;
  }

  return fmt::format("{}", f);
}

}  // namespace VW
