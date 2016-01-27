/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#include "example.h"
#include "unique_sort.h"

void unique_features(features& fs, int max)
{ if (fs.indicies_empty()) // REVIEW: is this fine with dense features?
    return;

  auto range = fs.values_indices_audit();
  auto last_index = range.begin();
  auto end = max > 0 ? range.begin() + min(fs.size(), (size_t)max) : range.end();

  for (auto i = ++range.begin(); i != end; ++i)
    if (i.index() != last_index.index())
      if (i != ++last_index)
      {
        last_index.value() = i.value();
        last_index.index() = i.index();
        if (!fs.space_names_empty())
          last_index.audit() = i.audit();
      }

  ++last_index;
  fs.truncate_to(last_index);
}

void unique_sort_features(uint64_t parse_mask, example* ae)
{
  for (auto ns : ae->indices)
    { features& fs = ae->feature_space[ns];
      if (fs.sort(parse_mask))
        unique_features(fs);
    }
  ae->sorted=true;
}
