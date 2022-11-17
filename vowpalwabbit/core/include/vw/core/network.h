// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

namespace VW
{
namespace details
{
int open_socket(const char* host, VW::io::logger& logger);
}
}  // namespace VW
