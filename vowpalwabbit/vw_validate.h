// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cstdint>

struct vw;

namespace VW
{
void validate_version(vw& all);
void validate_min_max_label(vw& all);
void validate_default_bits(vw& all, uint32_t local_num_bits);
void validate_num_bits(vw& all);
}  // namespace VW
