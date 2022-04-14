// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

#include <cstdint>

void unique_sort_features(uint64_t parse_mask, VW::example* ae);

void unique_features(features& fs, int max = -1);
