// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/io/logger.h"

namespace VW
{
namespace io
{
struct logger;
}
}  // namespace VW

int open_socket(const char* host, VW::io::logger& logger);
