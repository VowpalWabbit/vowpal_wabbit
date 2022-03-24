#pragma once

#include "vw_fwd.h"
#include "vowpalwabbit/vw_string_view.h"

namespace VW
{
bool try_extract_random_seed(const VW::example& ex, VW::string_view& view);
}  // namespace VW
