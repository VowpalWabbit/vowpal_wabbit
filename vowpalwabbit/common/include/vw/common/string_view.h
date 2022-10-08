// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "nonstd/string_view.h"

#include <ostream>

// This is a special case of the namespacing rules since it is a commonly used type.
namespace VW
{
using string_view = nonstd::string_view;

inline std::ostream& operator<<(std::ostream& stream, const string_view& str_view)
{
  nonstd::operator<<(stream, str_view);
  return stream;
}
}  // namespace VW
