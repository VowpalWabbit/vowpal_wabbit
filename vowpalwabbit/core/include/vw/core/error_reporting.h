// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"

#include <functional>
#include <string>

namespace VW
{
using trace_message_t = std::function<void(void*, const std::string&)>;
}

using trace_message_t VW_DEPRECATED(
    "Moved in VW namespace. Global symbol will be removed in VW 10.") = VW::trace_message_t;