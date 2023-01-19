// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/random.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/learner.h"

#include <queue>

namespace VW
{
namespace reductions
{
namespace multi_model
{
// ***** NOTE: ppw must be of form 2^n *****
inline void clear_offset(dense_parameters& weights, const size_t offset, const size_t ppw)
{
  assert(offset < ppw);

  for (auto iterator_clear = weights.begin() + offset; iterator_clear < weights.end(); iterator_clear += ppw)
  {
    assert((iterator_clear.index_without_stride() & (ppw - 1)) == offset);
    for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
    {
      *iterator_clear[stride_offset] = 0.0f;
    }
  }
}

// ***** NOTE: ppw must be of form 2^n *****
inline void resize_model_weights(
    dense_parameters& weights, const size_t offset, const size_t ppw, const size_t new_ppw_size)
{
  VW::weight* weights_arr = weights.first();
  const size_t refactor_size = ppw / new_ppw_size;
  for (size_t sub_model = 0; sub_model < new_ppw_size; ++sub_model)
  {
    for (size_t inner_ppw = 0; inner_ppw < refactor_size; ++inner_ppw)
    {
      if (inner_ppw != offset) { clear_offset(weights, sub_model * refactor_size + inner_ppw, ppw); }
    }
  }
  for (auto weights_it = weights.begin() + (offset * refactor_size); weights_it < weights.end(); weights_it += ppw)
  {
    uint32_t cb_ind = (weights_it.index() - (offset * refactor_size) * weights.stride()) / refactor_size;
    for (size_t ppw_offset = 0; ppw_offset < new_ppw_size; ++ppw_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        if (weights_arr[weights_it.index() + (ppw_offset * refactor_size) * weights.stride() + stride_offset] != 0.0f)
        {
          // Move all strided weights of selected model to smaller weights array. Zero out
          // previous weights.
          weights_arr[cb_ind + ppw_offset * weights.stride() + stride_offset] =
              weights_arr[weights_it.index() + (ppw_offset * refactor_size) * weights.stride() + stride_offset];
          if (weights_it.index() != 0 || ppw_offset != 0)
          {
            weights_arr[weights_it.index() + (ppw_offset * refactor_size) * weights.stride() + stride_offset] = 0.f;
          }
        }
      }
    }
  }
}
}  // namespace multi_model
}  // namespace reductions
}  // namespace VW