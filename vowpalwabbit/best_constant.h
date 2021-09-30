// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cfloat>
#include "vw.h"
#include "shared_data.h"

// TODO: This function is unsafe in daemon mode, which uses multiple processes
inline void count_label(shared_data* sd, float l)
{
  if (sd->is_more_than_two_labels_observed || l == FLT_MAX) return;

  float l1 = FLT_MAX;
  float l2 = FLT_MAX;
  if (!sd->first_observed_label.compare_exchange_strong(l1, l) && l1 != l)
  {
    if (!sd->second_observed_label.compare_exchange_strong(l2, l) && l2 != l)
    { sd->is_more_than_two_labels_observed = true; }
  }
}

bool get_best_constant(loss_function* loss_func, shared_data* sd, float& best_constant, float& best_constant_loss);
