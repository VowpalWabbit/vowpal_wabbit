// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/shared_data.h"
#include "vw/core/vw_fwd.h"

#include <cfloat>

namespace VW
{
inline void count_label(shared_data& sd, float l)
{
  if (sd.is_more_than_two_labels_observed || l == FLT_MAX) { return; }

  if (sd.first_observed_label != FLT_MAX)
  {
    if (sd.first_observed_label != l)
    {
      if (sd.second_observed_label != FLT_MAX)
      {
        if (sd.second_observed_label != l) { sd.is_more_than_two_labels_observed = true; }
      }
      else { sd.second_observed_label = l; }
    }
  }
  else { sd.first_observed_label = l; }
}

bool get_best_constant(
    const loss_function& loss_func, const shared_data& sd, float& best_constant, float& best_constant_loss);
}  // namespace VW
