// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

//! [Error Generator]
#define ERROR_CODE_DEFINITION(code, name, message) \
  namespace VW                                     \
  {                                                \
  namespace error_code                             \
  {                                                \
  const int name = code;                           \
  char const* const name##_s = message;            \
  }                                                \
  }
//! [Error Generator]

#include "errors_data.txt"

namespace VW
{
namespace error_code
{
// Success code
const int success = 0;
}  // namespace error_code
}  // namespace VW

namespace VW
{
namespace error_code
{
char const* const unknown_s = "Unexpected error.";
}
}  // namespace VW

#undef ERROR_CODE_DEFINITION
