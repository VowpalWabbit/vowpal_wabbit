// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <cfloat>
#include "vw.h"

static std::mutex count_label_mutex;

inline void count_label(shared_data* sd, float l)
{

  std::lock_guard<std::mutex> lck(count_label_mutex);

  if (sd->is_more_than_two_labels_observed.load() || l == FLT_MAX)
    return;

  if (sd->first_observed_label.load() != FLT_MAX)
  {
    if (sd->first_observed_label.load() != l)
    {
      if (sd->second_observed_label.load() != FLT_MAX)
      {
        if (sd->second_observed_label.load() != l)
          sd->is_more_than_two_labels_observed.store(true);
      }
      else
        sd->second_observed_label.store(l);
    }
  }
  else
    sd->first_observed_label.store(l);
}

bool get_best_constant(vw& all, float& best_constant, float& best_constant_loss);
