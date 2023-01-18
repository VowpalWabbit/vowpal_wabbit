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

  for (auto iterator_clear = weights.begin() + offset; iterator_clear < weights.end();
       iterator_clear += ppw)
  {
    assert((iterator_clear.index_without_stride() & (ppw - 1)) == offset);
    for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
    {
      *iterator_clear[stride_offset] = 0.0f;
    }
  }
}

// ***** NOTE: ppw must be of form 2^n *****
inline void resize_model_weights(dense_parameters& weights, const size_t offset, const size_t ppw, const size_t new_ppw_size)
{
  VW::weight* weights_arr = weights.first();
  const size_t new_ppw_count = ppw / new_ppw_size;
  for (auto sub_model = 0; sub_model < new_ppw_count; ++sub_model)
  {
    for (auto inner_ppw = 0; inner_ppw < new_ppw_size; ++inner_ppw)
    {
      if (sub_model != offset) { clear_offset(weights, sub_model * new_ppw_size + inner_ppw, ppw); }
    }
  }
  for (auto weights_it = weights.begin() + new_ppw_size * offset; weights_it < weights.end(); weights_it += ppw)
  {
    if (weights_it.index() == 0) { continue; }  // Index is same for 0
    uint32_t cb_ind = (weights_it.index() - new_ppw_size * offset * weights.stride()) / new_ppw_count;
    for (auto ppw_offset = 0; ppw_offset < new_ppw_size; ++ppw_offset)
    {
      for (auto stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        if (weights_arr[weights_it.index() + ppw_offset * weights.stride() + stride_offset] != 0.0f)
        {
          // Move all strided weights of selected model to smaller weights array. Zero out
          // previous weights.
          weights_arr[cb_ind + ppw_offset * weights.stride() + stride_offset] = weights_arr[weights_it.index() + ppw_offset * weights.stride() + stride_offset];
          weights_arr[weights_it.index() + ppw_offset * weights.stride() + stride_offset] = 0.f;
        }
      }
    }
  }
}
}  // namespace multi_model
}  // namespace reductions
}  // namespace VW