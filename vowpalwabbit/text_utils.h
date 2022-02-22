// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>

#include "vw_string_view.h"
#include "io/logger.h"

namespace VW
{
/**
 * \brief Check if a string ends with some other string.
 * \param full_string String to check ending of
 * \param ending Ending value to check
 * \return true if full_string ends with ending, otherwise false.
 */
bool ends_with(VW::string_view full_string, VW::string_view ending);

/**
 * \brief Replace hex sequences in a string with their corresponding byte. A hex sequence must only contain two digits
 * and must be in the form \x00 \param arg String to replace hex values within \return A copy of the original string
 * with hex values replaced with corresponding byte.
 */
std::string decode_inline_hex(VW::string_view arg, VW::io::logger& logger);

/**
 * @brief Wrap text by whole words with the given column width.
 *
 * @param text text to wrap
 * @param width column width to wrap to
 * @param wrap_after if word causes line to exceed width include word on same line. If false, this word would be wrapped
 * to the next line.
 * @return std::string copy of string with required newlines
 */
std::string wrap_text(VW::string_view text, size_t width, bool wrap_after = true);

}  // namespace VW
