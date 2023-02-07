// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"

#include <string>

namespace VW
{
// This must be a raw function pointer for use in C# bindings
using trace_message_t = void (*)(void*, const std::string&);
}  // namespace VW

using trace_message_t VW_DEPRECATED(
    "Moved in VW namespace. Global symbol will be removed in VW 10.") = VW::trace_message_t;