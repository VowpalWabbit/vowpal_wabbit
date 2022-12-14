// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/array_parameters_dense.h"
#include "vw/core/learner.h"
#include "vw/core/rand_state.h"

#include <queue>

namespace VW
{
namespace reductions
{
namespace multi_model
{
// ***** NOTE: params_per_problem must be of form 2^n *****
inline void clear_offset(dense_parameters& weights, const size_t offset, const size_t params_per_problem)
{
  assert(offset < params_per_problem);

  for (auto iterator_clear = weights.begin() + offset; iterator_clear < weights.end();
       iterator_clear += params_per_problem)
  {
    assert((iterator_clear.index_without_stride() & (params_per_problem - 1)) == offset);
    for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
    {
      *iterator_clear[stride_offset] = 0.0f;
    }
  }
}

// ***** NOTE: params_per_problem must be of form 2^n *****
inline void adjust_weights_single_model(dense_parameters& weights, const size_t offset, const size_t params_per_problem)
{
  for (size_t i = 0; i < params_per_problem; ++i)
  {
    if (i != offset) { clear_offset(weights, i, params_per_problem); }
  }
  for (auto weights_it = weights.begin() + offset; weights_it < weights.end(); weights_it += params_per_problem)
  {
    if (weights_it.index() == 0) { continue; }  // Index is same for 0
    uint32_t cb_ind = weights_it.index() / params_per_problem;
    for (uint64_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
    {
      if (*weights_it[stride_offset] != 0.0f)
      {
        // Move all strided weights of selected model to smaller weights array. Zero out
        // previous weights.
        weights.first()[cb_ind + stride_offset] = *weights_it[stride_offset];
        *weights_it[stride_offset] = 0.f;
      }
    }
  }
}
}  // namespace multi_model
}  // namespace reductions
}  // namespace VW