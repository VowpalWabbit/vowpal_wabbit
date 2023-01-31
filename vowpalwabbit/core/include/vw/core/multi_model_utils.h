// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/random.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/learner.h"

namespace VW
{
namespace reductions
{
namespace multi_model
{
// ***** NOTE: overall_ppw_size must be of form 2^n *****
inline void clear_innermost_offset(
    dense_parameters& weights, const size_t offset, const size_t overall_ppw_size, const size_t innermost_ppw_size)
{
  VW::weight* weights_arr = weights.first();
  const size_t overall_without_innermost_ppw_size = overall_ppw_size / innermost_ppw_size;
  assert(offset < innermost_ppw_size);

  for (auto iterator_clear = weights.begin(); iterator_clear < weights.end(); iterator_clear += overall_ppw_size)
  {
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_ppw_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        weights_arr[iterator_clear.index() + outer_offset * innermost_ppw_size * weights.stride() +
            offset * weights.stride() + stride_offset] = 0.0f;
      }
    }
  }
}

// ***** NOTE: overall_ppw_size must be of form 2^n *****
inline void move_innermost_offsets(dense_parameters& weights, const size_t from, const size_t to,
    const size_t overall_ppw_size, const size_t innermost_ppw_size, bool swap = false)
{
  VW::weight* weights_arr = weights.first();
  const size_t overall_without_innermost_ppw_size = overall_ppw_size / innermost_ppw_size;
  assert(from < innermost_ppw_size);
  assert(to < innermost_ppw_size);

  for (auto iterator_move = weights.begin(); iterator_move < weights.end(); iterator_move += overall_ppw_size)
  {
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_ppw_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); stride_offset++)
      {
        size_t outer_index =
            iterator_move.index() + outer_offset * innermost_ppw_size * weights.stride() + stride_offset;
        if (weights_arr[outer_index + to * weights.stride()] != weights_arr[outer_index + from * weights.stride()])
        {
          if (swap)
          {
            std::swap(
                weights_arr[outer_index + to * weights.stride()], weights_arr[outer_index + from * weights.stride()]);
          }
          else
          {
            weights_arr[outer_index + to * weights.stride()] = weights_arr[outer_index + from * weights.stride()];
          }
        }
      }
    }
  }
}

// ***** NOTE: overall_ppw_size must be of form 2^n *****
inline void reduce_innermost_model_weights(
    dense_parameters& weights, const size_t offset, const size_t overall_ppw_size, const size_t innermost_ppw_size)
{
  VW::weight* weights_arr = weights.first();
  const size_t overall_without_innermost_ppw_size = overall_ppw_size / innermost_ppw_size;
  for (size_t inner_ppw = 0; inner_ppw < innermost_ppw_size; ++inner_ppw)
  {
    if (inner_ppw != offset) { clear_innermost_offset(weights, inner_ppw, overall_ppw_size, innermost_ppw_size); }
  }
  for (auto weights_it = weights.begin(); weights_it < weights.end(); weights_it += overall_ppw_size)
  {
    uint32_t cb_ind = weights_it.index() / innermost_ppw_size;
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_ppw_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        auto old_ind = weights_it.index() + outer_offset * innermost_ppw_size * weights.stride() +
            offset * weights.stride() + stride_offset;
        auto new_ind = cb_ind + outer_offset * weights.stride() + stride_offset;
        if (weights_arr[old_ind] != 0.0f)
        {
          // Move all strided weights of selected model to smaller weights array. Zero out
          // previous weights.
          weights_arr[new_ind] = weights_arr[old_ind];
          if (weights_it.index() != 0 || outer_offset != 0) { weights_arr[old_ind] = 0.f; }
        }
      }
    }
  }
}
}  // namespace multi_model
}  // namespace reductions
}  // namespace VW