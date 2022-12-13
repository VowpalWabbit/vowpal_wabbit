// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace details
{
void binary_print_result_by_ref(
    VW::io::writer* f, float res, float weight, const VW::v_array<char>& tag, VW::io::logger& logger);

void get_prediction(VW::io::reader* f, float& res, float& weight);
}  // namespace details
}  // namespace VW