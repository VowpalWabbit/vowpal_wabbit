// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdint>

namespace VW
{
class workspace;
}

namespace VW
{
void validate_version(VW::workspace& all);
void validate_min_max_label(VW::workspace& all);
void validate_default_bits(VW::workspace& all, uint32_t local_num_bits);
void validate_num_bits(VW::workspace& all);
}  // namespace VW
