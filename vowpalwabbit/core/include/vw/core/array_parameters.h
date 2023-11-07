// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/array_parameters_dense.h"
#include "vw/core/array_parameters_sparse.h"

namespace VW
{
class parameters
{
public:
  bool adaptive;
  bool normalized;

  bool sparse;
  dense_parameters dense_weights;
  sparse_parameters sparse_weights;

  inline VW::weight& operator[](size_t i)
  {
    if (sparse) { return sparse_weights[i]; }
    else { return dense_weights[i]; }
  }

  template <typename Lambda>
  void set_default(Lambda&& default_func)
  {
    if (sparse) { sparse_weights.set_default(std::forward<Lambda>(default_func)); }
    else { dense_weights.set_default(std::forward<Lambda>(default_func)); }
  }

  inline uint32_t stride_shift() const
  {
    if (sparse) { return sparse_weights.stride_shift(); }
    else { return dense_weights.stride_shift(); }
  }

  inline uint64_t stride() const
  {
    if (sparse) { return sparse_weights.stride(); }
    else { return dense_weights.stride(); }
  }

  inline uint64_t mask() const
  {
    if (sparse) { return sparse_weights.mask(); }
    else { return dense_weights.mask(); }
  }

  inline void shallow_copy(const parameters& input)
  {
    if (sparse) { sparse_weights.shallow_copy(input.sparse_weights); }
    else { dense_weights = VW::dense_parameters::shallow_copy(input.dense_weights); }
  }

  inline void set_zero(size_t offset)
  {
    if (sparse) { sparse_weights.set_zero(offset); }
    else { dense_weights.set_zero(offset); }
  }
#ifndef _WIN32
#  ifndef DISABLE_SHARED_WEIGHTS
  inline void share(size_t length)
  {
    if (sparse) { sparse_weights.share(length); }
    else { dense_weights.share(length); }
  }
#  endif
#endif

  inline void stride_shift(uint32_t stride_shift)
  {
    if (sparse) { sparse_weights.stride_shift(stride_shift); }
    else { dense_weights.stride_shift(stride_shift); }
  }

  inline VW::weight& strided_index(size_t index)
  {
    if (sparse) { return sparse_weights.strided_index(index); }
    else { return dense_weights.strided_index(index); }
  }

  inline bool not_null()
  {
    if (sparse) { return sparse_weights.not_null(); }
    else { return dense_weights.not_null(); }
  }
};
}  // namespace VW

using parameters VW_DEPRECATED("parameters moved into VW namespace") = VW::parameters;
