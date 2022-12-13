// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

#include <cstdint>

namespace VW
{
/**
 * \brief Remove all non unique features from a feature group.
 * - Uniqueness is determined by feature index.
 * - This function requires the feature group to be sorted. For sorting see features::sort.
 *
 * \param fs Feature group to remove non-unique features from
 * \param max The maximum number of unique features to keep. -1 to keep all unique features.
 */
void unique_features(features& fs, int max = -1);

void unique_sort_features(uint64_t parse_mask, VW::example& ae);
}  // namespace VW
