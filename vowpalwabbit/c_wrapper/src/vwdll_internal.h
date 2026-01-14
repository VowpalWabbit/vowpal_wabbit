// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <string>

// Internal helper function for UTF-16 to UTF-8 conversion
// This is not part of the public C API
std::string utf16_to_utf8(const std::u16string& utf16_string);
