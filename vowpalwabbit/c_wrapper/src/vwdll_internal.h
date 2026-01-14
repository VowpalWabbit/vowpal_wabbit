// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include <string>

// Internal helper function for UTF-16 to UTF-8 conversion
// This replaces the deprecated std::wstring_convert and std::codecvt_utf8_utf16
inline std::string utf16_to_utf8(const std::u16string& utf16_string)
{
  std::string utf8_string;
  utf8_string.reserve(utf16_string.size() * 3);  // Reserve space for worst case

  for (size_t i = 0; i < utf16_string.size(); ++i)
  {
    uint32_t codepoint = utf16_string[i];

    // Handle surrogate pairs (for codepoints > U+FFFF)
    if (codepoint >= 0xD800 && codepoint <= 0xDBFF && i + 1 < utf16_string.size())
    {
      uint32_t low = utf16_string[i + 1];
      if (low >= 0xDC00 && low <= 0xDFFF)
      {
        codepoint = ((codepoint - 0xD800) << 10) + (low - 0xDC00) + 0x10000;
        ++i;  // Skip the low surrogate
      }
    }

    // Encode as UTF-8
    if (codepoint < 0x80)
    {
      // 1-byte sequence (ASCII)
      utf8_string.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint < 0x800)
    {
      // 2-byte sequence
      utf8_string.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
      utf8_string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint < 0x10000)
    {
      // 3-byte sequence
      utf8_string.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
      utf8_string.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
      utf8_string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    else if (codepoint < 0x110000)
    {
      // 4-byte sequence
      utf8_string.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
      utf8_string.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
      utf8_string.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
      utf8_string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    // Invalid codepoints are silently skipped
  }

  return utf8_string;
}
