// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

// TODO: Make simd work with MSVC. Only works on linux for now.
// TODO: Only works for x86. Make simd work on other architectures e.g. using SIMDe.
#ifdef BUILD_LAS_WITH_SIMD

#  include "vw/core/example.h"
#  include "vw/core/global_data.h"

namespace VW
{
namespace cb_explore_adf
{
inline bool cpu_supports_avx2() { return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma"); }

inline bool cpu_supports_avx512()
{
  return __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512bw") &&
      __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("avx512vpopcntdq");
}

// A data parallel implementation of the foreach_feature that processes 8 features at once.
float compute_dot_prod_avx2(uint64_t column_index, VW::workspace* _all, uint64_t seed, VW::example* ex);

// A data parallel implementation of the foreach_feature that processes 16 features at once.
float compute_dot_prod_avx512(uint64_t column_index, VW::workspace* _all, uint64_t seed, VW::example* ex);

}  // namespace cb_explore_adf
}  // namespace VW

#endif
