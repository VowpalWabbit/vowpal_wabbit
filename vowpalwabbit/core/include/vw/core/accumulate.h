// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

// This implements various accumulate functions building on top of allreduce.
#pragma once

#include "vw/core/vw_fwd.h"

#include <cstddef>
#include <cstdint>

namespace VW
{
namespace details
{
void accumulate(VW::workspace& all, parameters& weights, size_t o);
float accumulate_scalar(VW::workspace& all, float local_sum);
void accumulate_weighted_avg(VW::workspace& all, parameters& weights);
void accumulate_avg(VW::workspace& all, parameters& weights, size_t o);
template <class T>
void do_weighting(size_t normalized_idx, uint64_t length, const float* local_weights, T& weights)
{
  for (uint64_t i = 0; i < length; i++)
  {
    float* weight = &weights.strided_index(i);
    if (local_weights[i] > 0)
    {
      const float ratio = weight[1] / local_weights[i];
      weight[0] *= ratio;
      weight[1] *= ratio;  // A crude max
      if (normalized_idx > 0)
      {
        weight[normalized_idx] *= ratio;  // A crude max
      }
    }
    else { *weight = 0; }
  }
}

}  // namespace details
}  // namespace VW