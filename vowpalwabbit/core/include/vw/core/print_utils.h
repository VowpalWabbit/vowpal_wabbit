// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"

#include <memory>
#include <vector>

namespace VW
{
namespace details
{
void global_print_newline(
    const std::vector<std::unique_ptr<VW::io::writer>>& final_prediction_sink, VW::io::logger& logger);
}
}  // namespace VW
