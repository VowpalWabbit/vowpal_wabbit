// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "text_utils.h"
#include "io/logger.h"

#include <sstream>

namespace VW
{
bool ends_with(VW::string_view full_string, VW::string_view ending) { return full_string.ends_with(ending); }

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

std::string wrap_text(VW::string_view text, size_t width)
{
  std::stringstream ss;
  size_t start_line = 0;
  size_t pos = 0;
  std::string delim = "";
  while (pos < text.size())
  {
    while (pos < text.size() && ((text[pos] != ' ') || (pos - start_line) < width)) { pos++; }
    if (pos > text.size()) { pos = text.size(); }
    ss << delim << text.substr(start_line, pos - start_line);
    delim = "\n";
    pos += 1;
    start_line = pos;
  }
  return ss.str();
}

}  // namespace VW
