// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/errno_handling.h"

#include "vw/common/vw_exception.h"

#include <array>
#include <cstring>
#include <string>

#ifndef _WIN32
#  include <clocale>
#endif

std::string VW::io::strerror_to_string(int error_number)
{
#ifdef _WIN32
  static constexpr auto BUFFER_SIZE = 256;
  std::array<char, BUFFER_SIZE> error_message_buffer;
  auto result = strerror_s(error_message_buffer.data(), error_message_buffer.size() - 1, error_number);
  if (result != 0) { return "unknown message for errno: " + std::to_string(error_number); }

  auto length = std::strlen(error_message_buffer.data());
  return std::string(error_message_buffer.data(), length);
#elif __APPLE__
  static constexpr auto BUFFER_SIZE = 256;
  std::array<char, BUFFER_SIZE> error_message_buffer;
#  if defined(__GLIBC__) && defined(_GNU_SOURCE)
  // You must use the returned buffer and not the passed in buffer the GNU version.
  char* message_buffer = strerror_r(error_number, error_message_buffer.data(), error_message_buffer.size() - 1);
  auto length = std::strlen(message_buffer);
  return std::string(message_buffer, length);
#  else
  auto result = strerror_r(error_number, error_message_buffer.data(), error_message_buffer.size() - 1);
  if (result != 0) { return "unknown message for errno: " + std::to_string(error_number); }
  auto length = std::strlen(error_message_buffer.data());
  return std::string(error_message_buffer.data(), length);
#  endif
#else
  // Passing "" for the locale means use the default system locale
  locale_t locale = newlocale(LC_ALL_MASK, "", static_cast<locale_t>(nullptr));

  if (locale == static_cast<locale_t>(nullptr))
  {
    return "Failed to create locale when getting error message for errno: " + std::to_string(error_number);
  }

  // Even if error_number is unknown, will return a "Unknown error nnn" message.
  std::string message = strerror_l(error_number, locale);
  freelocale(locale);
  return message;
#endif
}
