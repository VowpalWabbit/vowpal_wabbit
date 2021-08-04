// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "example.h"
#include "unique_sort.h"
#include "feature_group.h"

#include <algorithm>

/**
 * \brief Remove all non unique features from a feature group.
 * - Uniqueness is determined by feature index.
 * - This function requires the feature group to be sorted. For sorting see features::sort.
 *
 * \param fs Feature group to remove non-unique features from
 * \param max The maximum number of unique features to keep. -1 to keep all unique features.
 */
void unique_features(features& fs, int max)
{
  if (fs.indicies.empty()) return;
  if (max == 0)
  {
    fs.clear();
    return;
  }
  if (max == 1)
  {
    fs.truncate_to(1);
    return;
  }

  auto last_index = std::size_t{0};
  for (auto i = std::size_t{1}; i != fs.size(); ++i)
  {
    if (fs.indicies[i] != fs.indicies[last_index])
    {
      if (i != ++last_index)
      {
        fs.values[last_index] = fs.values[i];
        fs.indicies[last_index] = fs.indicies[i];
        if (!fs.space_names.empty()) { fs.space_names[last_index] = std::move(fs.space_names[i]); }
      }

      const auto unique_items_found = last_index + 1;
      if (static_cast<int>(unique_items_found) >= max) { break; }
    }
  }

  ++last_index;
  fs.truncate_to(last_index);
}

void unique_sort_features(uint64_t parse_mask, example* ae)
{
  for (features& fs : *ae)
  {
    if (fs.sort(parse_mask)) { unique_features(fs); }
  }

  ae->sorted = true;
}
