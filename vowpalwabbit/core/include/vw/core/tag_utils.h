#pragma once

#include "vw/common/string_view.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
bool try_extract_random_seed(const VW::example& ex, VW::string_view& view);
}  // namespace VW
