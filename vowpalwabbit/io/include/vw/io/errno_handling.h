// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/vw_exception.h"

#include <sstream>
#include <string>

namespace VW
{
namespace io
{
std::string strerror_to_string(int error_number);
}  // namespace io
}  // namespace VW

#define THROWERRNO(args)                                        \
  {                                                             \
    std::ostringstream __msg;                                   \
    __msg << args;                                              \
    __msg << ", errno = " << VW::io::strerror_to_string(errno); \
    throw VW::vw_exception(VW_FILENAME, __LINE__, __msg.str()); \
  }
