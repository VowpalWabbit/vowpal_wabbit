// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/vw_fwd.h"

#include <string>

VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_BADLY_FORMED_XML

namespace VW
{
/**
 * \brief Replace hex sequences in a string with their corresponding byte. A hex sequence must only contain two digits
 * and must be in the form \x00 \param arg String to replace hex values within \return A copy of the original string
 * with hex values replaced with corresponding byte.
 */
std::string decode_inline_hex(VW::string_view arg, VW::io::logger& logger);

/**
 * @brief Format float to string with max number of digits after the decimal place
 *
 * @param f float to format
 * @param max_decimal_places if >=0 the max number of digits after decimal place. If <0, then as many digits are needed
 * to represent the number will be used
 * @return std::string formatted float
 */
std::string fmt_float(float f, int max_decimal_places);

}  // namespace VW

VW_WARNING_STATE_POP