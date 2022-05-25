// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#define ERROR_CODE_DEFINITION(code, name, message) \
  namespace VW                                     \
  {                                                \
  namespace experimental                           \
  {                                                \
  namespace error_code                             \
  {                                                \
  constexpr int name = code;                       \
  char const* const name##_s = message;            \
  }                                                \
  }                                                \
  }

#include "error_data.h"

namespace VW
{
namespace experimental
{
namespace error_code
{
// Success code
constexpr int success = 0;
}  // namespace error_code
}  // namespace experimental
}  // namespace VW

namespace VW
{
namespace experimental
{
namespace error_code
{
char const* const unknown_s = "Unexpected error.";
}
}  // namespace experimental
}  // namespace VW

#undef ERROR_CODE_DEFINITION