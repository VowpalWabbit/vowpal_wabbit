// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "debug_log.h"
#include "interactions_predict.h"
#include "v_array.h"
#include "example_predict.h"

#undef VW_DEBUG_LOG
#define VW_DEBUG_LOG vw_dbg::gd_predict

namespace GD
{
// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_index)
template <class DataT, void (*FuncT)(DataT&, float feature_value, uint64_t feature_index), bool privacy_activation>
void foreach_feature(const features& fs, DataT& dat, uint64_t offset = 0, float mult = 1.)
{
  for (const auto& f : fs) { FuncT(dat, mult * f.value(), f.index() + offset); }
}

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_index)
template <class DataT, void (*FuncT)(DataT&, float feature_value, uint64_t feature_index), class WeightsT, bool privacy_activation>
void foreach_feature(WeightsT& /*weights*/, const features& fs, DataT& dat, uint64_t offset = 0, float mult = 1.)
{
  for (const auto& f : fs) { FuncT(dat, mult * f.value(), f.index() + offset); }
}

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, void (*FuncT)(DataT&, const float feature_value, float& weight_reference), class WeightsT, bool privacy_activation>
inline void foreach_feature(WeightsT& weights, const features& fs, DataT& dat, uint64_t offset = 0, float mult = 1.)
{
  for (const auto& f : fs)
  {
    if VW_STD17_CONSTEXPR (privacy_activation)
    {
      weights.set_privacy_preserving_bit(f.index() + offset);
    }
    weight& w = weights[(f.index() + offset)];
    FuncT(dat, mult * f.value(), w);
  }
}

// iterate through one namespace (or its part), callback function FuncT(some_data_R, feature_value_x, feature_weight)
template <class DataT, void (*FuncT)(DataT&, float, float), class WeightsT, bool privacy_activation>
inline void foreach_feature(
    const WeightsT& weights, const features& fs, DataT& dat, uint64_t offset = 0, float mult = 1.)
{
  for (const auto& f : fs) { FuncT(dat, mult * f.value(), weights[(f.index() + offset)]); }
}

template <class DataT>
inline void dummy_func(DataT&, const audit_strings*)
{
}  // should never be called due to call_audit overload

template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT),
    class WeightsT, bool privacy_activation>  // nullptr func can't be used as template param in old
                     // compilers

inline void generate_interactions(const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, example_predict& ec,
    DataT& dat, WeightsT& weights, size_t& num_interacted_features,
    INTERACTIONS::generate_interactions_object_cache& cache)  // default value removed to eliminate
                                                              // ambiguity in old complers
{
  INTERACTIONS::generate_interactions<DataT, WeightOrIndexT, FuncT, false, dummy_func<DataT>, WeightsT, privacy_activation>(
      interactions, extent_interactions, permutations, ec, dat, weights, num_interacted_features, cache);
}

// iterate through all namespaces and quadratic&cubic features, callback function FuncT(some_data_R, feature_value_x,
// WeightOrIndexT) where WeightOrIndexT is EITHER float& feature_weight OR uint64_t feature_index
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT), class WeightsT, bool privacy_activation>
inline void foreach_feature(WeightsT& weights, bool ignore_some_linear, std::array<bool, NUM_NAMESPACES>& ignore_linear,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, example_predict& ec,
    DataT& dat, size_t& num_interacted_features, INTERACTIONS::generate_interactions_object_cache& cache)
{
  uint64_t offset = ec.ft_offset;
  if (ignore_some_linear)
    for (example_predict::iterator i = ec.begin(); i != ec.end(); ++i)
    {
      if (!ignore_linear[i.index()])
      {
        features& f = *i;
        foreach_feature<DataT, FuncT, WeightsT, privacy_activation>(weights, f, dat, offset);
      }
    }
  else
    for (features& f : ec) foreach_feature<DataT, FuncT, WeightsT, privacy_activation>(weights, f, dat, offset);

  generate_interactions<DataT, WeightOrIndexT, FuncT, WeightsT, privacy_activation>(
      interactions, extent_interactions, permutations, ec, dat, weights, num_interacted_features, cache);
}

template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT), class WeightsT, bool privacy_activation>
inline void foreach_feature(WeightsT& weights, bool ignore_some_linear, std::array<bool, NUM_NAMESPACES>& ignore_linear,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, example_predict& ec,
    DataT& dat, INTERACTIONS::generate_interactions_object_cache& cache)
{
  size_t num_interacted_features_ignored = 0;
  foreach_feature<DataT, WeightOrIndexT, FuncT, WeightsT, privacy_activation>(weights, ignore_some_linear, ignore_linear, interactions,
      extent_interactions, permutations, ec, dat, num_interacted_features_ignored, cache);
}

inline void vec_add(float& p, float fx, float fw) { p += fw * fx; }

template <class WeightsT>
inline float inline_predict(WeightsT& weights, bool ignore_some_linear, std::array<bool, NUM_NAMESPACES>& ignore_linear,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, example_predict& ec,
    INTERACTIONS::generate_interactions_object_cache& cache, float initial = 0.f)
{
  foreach_feature<float, float, vec_add, WeightsT, false>(
      weights, ignore_some_linear, ignore_linear, interactions, extent_interactions, permutations, ec, initial, cache);
  return initial;
}

template <class WeightsT>
inline float inline_predict(WeightsT& weights, bool ignore_some_linear, std::array<bool, NUM_NAMESPACES>& ignore_linear,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions, bool permutations, example_predict& ec,
    size_t& num_interacted_features, INTERACTIONS::generate_interactions_object_cache& cache, float initial = 0.f)
{
  foreach_feature<float, float, vec_add, WeightsT, false>(weights, ignore_some_linear, ignore_linear, interactions,
      extent_interactions, permutations, ec, initial, num_interacted_features, cache);
  return initial;
}
}  // namespace GD
