// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/gd_predict.h"
#include "vw/core/interactions_predict.h"

namespace VW
{
namespace details
{
template <typename WeightsT, typename FuncT>
inline void foreach_feature_inner(
    const features& feature_group, WeightsT& weights, const FuncT& func, uint64_t offset = 0, float mult = 1.)
{
  for (const auto& feat : feature_group)
  { func(feat.index() + offset, mult * feat.value(), weights[static_cast<size_t>(feat.index() + offset)]); }
}
template <typename WeightsT, typename FuncT>
inline void foreach_linear_feature(const VW::example_predict& ec, WeightsT& weights, bool ignore_some_linear,
    const std::array<bool, NUM_NAMESPACES>& ignore_linear, const FuncT& func)
{
  uint64_t offset = ec.ft_offset;
  if (ignore_some_linear)
  {
    for (auto i = ec.begin(); i != ec.end(); ++i)
    {
      if (!ignore_linear[i.index()]) { foreach_feature_inner(*i, weights, func, offset); }
    }
  }
  else
  {
    for (const features& feats : ec) { foreach_feature_inner(feats, weights, func, offset); }
  }
}

inline void unused_dummy_audit_func(const VW::audit_strings* /*unused*/)
{
  // should never be called due to call_audit overload
  assert(false);
}

template <typename WeightsT, typename FuncT>
inline void foreach_interacted_feature(const VW::example_predict& example, WeightsT& weights,
    const std::vector<std::vector<VW::namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, size_t& num_features,
    INTERACTIONS::generate_interactions_object_cache& cache, const FuncT& func)
{
  num_features = 0;
  // often used values
  const auto inner_kernel_func = [&](features::const_audit_iterator begin, features::const_audit_iterator end,
                                     feature_value value, feature_index index) {
    INTERACTIONS::details::inner_kernel_v2<false>(
        weights, begin, end, example.ft_offset, value, index, func, details::unused_dummy_audit_func);
  };
  INTERACTIONS::details::generate_interactions_impl<false>(interactions, extent_interactions, permutations, example,
      num_features, cache, inner_kernel_func, details::unused_dummy_audit_func);
}

}  // namespace details

template <typename WeightsT, typename FuncT, typename AuditFuncT>
inline void audit_foreach_interacted_feature(const VW::example_predict& ec, WeightsT& weights,
    const std::vector<std::vector<VW::namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, size_t& num_features,
    INTERACTIONS::generate_interactions_object_cache& cache, const FuncT& func, const AuditFuncT& audit_func)
{
  num_features = 0;
  // often used values
  const auto inner_kernel_func = [&](features::const_audit_iterator begin, features::const_audit_iterator end,
                                     feature_value value, feature_index index) {
    INTERACTIONS::details::inner_kernel_v2<true>(weights, begin, end, ec.ft_offset, value, index, func, audit_func);
  };
  const auto depth_audit_func = [&](const VW::audit_strings* audit_str) { audit_func(audit_str); };
  INTERACTIONS::details::generate_interactions_impl<true>(
      interactions, extent_interactions, permutations, ec, num_features, cache, inner_kernel_func, depth_audit_func);
}

template <typename WeightsT, typename FuncT>
inline void foreach_feature(const VW::example_predict& example, WeightsT& weights, bool ignore_some_linear,
    std::array<bool, NUM_NAMESPACES>& ignore_linear, const std::vector<std::vector<VW::namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations,
    size_t& num_interacted_features, INTERACTIONS::generate_interactions_object_cache& cache, const FuncT func)
{
  num_interacted_features = 0;
  details::foreach_linear_feature(example, weights, ignore_some_linear, ignore_linear, func);
  details::foreach_interacted_feature(
      example, weights, interactions, extent_interactions, permutations, num_interacted_features, cache, func);
}
}  // namespace VW