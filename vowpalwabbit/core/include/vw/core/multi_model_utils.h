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
/*
***** NOTE: total_feature_width must be of form 2^n for all functions *****
These functions are used to maniputate the weights of a multi-model learner. As an example of what each variable
is, consider a learner with the command line "--bag 4 --automl 2". In this scenario 'automl' is above 'bag' in the
stack, and would be considered to have the innermost feature_widths. Thus innermost_feature_width_size = 2. The
total_feature_width = 8 here as it is the product of all feature_width's in the stack (4 * 2). The actual weights here
will look like this:

0     1     2     3      4      5      6      7       -- Indices as printed using --invert_hash
0     4     8     12     16     20     24     28      -- Actual weight indices included stride size of 4
bag0  bag0  bag1  bag1   bag2   bag2   bag3   bag3    -- Shows how weights are grouped by bag learner number
aml0  aml1  aml0  aml1   aml0   aml1   aml0   aml1    -- Shows how weights are grouped by automl learner number
*/

// This function is used to clear the weights of a specific offset in the innermost feature_width.
inline void clear_innermost_offset(dense_parameters& weights, const size_t offset, const size_t total_feature_width,
    const size_t innermost_feature_width_size)
{
  VW::weight* weights_arr = weights.data();
  const size_t overall_without_innermost_feature_width_size = total_feature_width / innermost_feature_width_size;
  assert(offset < innermost_feature_width_size);

  for (auto iterator_clear = weights.begin(); iterator_clear < weights.end(); iterator_clear += total_feature_width)
  {
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_feature_width_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        weights_arr[iterator_clear.index() + outer_offset * innermost_feature_width_size * weights.stride() +
            offset * weights.stride() + stride_offset] = 0.0f;
      }
    }
  }
}

// This function is used to move the weights of a specific offset in the innermost feature_width to another offset.
inline void move_innermost_offsets(dense_parameters& weights, const size_t from, const size_t to,
    const size_t total_feature_width, const size_t innermost_feature_width_size, bool swap = false)
{
  VW::weight* weights_arr = weights.data();
  const size_t overall_without_innermost_feature_width_size = total_feature_width / innermost_feature_width_size;
  assert(from < innermost_feature_width_size);
  assert(to < innermost_feature_width_size);

  for (auto iterator_move = weights.begin(); iterator_move < weights.end(); iterator_move += total_feature_width)
  {
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_feature_width_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); stride_offset++)
      {
        size_t outer_index =
            iterator_move.index() + outer_offset * innermost_feature_width_size * weights.stride() + stride_offset;
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

/* This function is used to select one sub-offset within the innermost feature_width and remove all others. For instance
 if this called with "--bag 4 --automl 2" using offset = 1 (we will have innermost_feature_width_size = 2 and
 total_feature_width = 8) then this will remove all weights with automl = 0. The set of weights referenced above:

  0     1     2     3      4      5      6      7
  0     4     8     12     16     20     24     28
  bag0  bag0  bag1  bag1   bag2   bag2   bag3   bag3
  aml0  aml1  aml0  aml1   aml0   aml1   aml0   aml1

  will be reduced to:

  0     1     2     3
  0     4     8     12
  bag0  bag1  bag2  bag3
  aml1  aml1  aml1  aml1
*/
inline void reduce_innermost_model_weights(dense_parameters& weights, const size_t offset,
    const size_t total_feature_width, const size_t innermost_feature_width_size)
{
  VW::weight* weights_arr = weights.data();
  const size_t overall_without_innermost_feature_width_size = total_feature_width / innermost_feature_width_size;
  for (size_t inner_feature_widths = 0; inner_feature_widths < innermost_feature_width_size; ++inner_feature_widths)
  {
    if (inner_feature_widths != offset)
    {
      clear_innermost_offset(weights, inner_feature_widths, total_feature_width, innermost_feature_width_size);
    }
  }
  for (auto weights_it = weights.begin(); weights_it < weights.end(); weights_it += total_feature_width)
  {
    uint32_t cb_ind = weights_it.index() / innermost_feature_width_size;
    for (size_t outer_offset = 0; outer_offset < overall_without_innermost_feature_width_size; ++outer_offset)
    {
      for (size_t stride_offset = 0; stride_offset < weights.stride(); ++stride_offset)
      {
        auto old_ind = weights_it.index() + outer_offset * innermost_feature_width_size * weights.stride() +
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