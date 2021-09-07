// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include <cstdint>
#include "constant.h"
#include "feature_group.h"
#include "example_predict.h"
#include <vector>
#include <string>

const static std::pair<std::string, std::string> EMPTY_AUDIT_STRINGS = std::make_pair("", "");

namespace INTERACTIONS
{
using features_range_t = std::pair<features::const_audit_iterator, features::const_audit_iterator>;

/*
 * By default include interactions of feature with itself.
 * This approach produces slightly more interactions but it's safer
 * for some cases, as discussed in issues/698
 * Previous behaviour was: include interactions of feature with itself only if its value != value^2.
 *
 */

// 3 template functions to pass FuncT() proper argument (feature idx in regressor, or its coefficient)

template <class DataT, void (*FuncT)(DataT&, const float, float&), class WeightsT>
inline void call_FuncT(DataT& dat, WeightsT& weights, const float ft_value, const uint64_t ft_idx)
{
  FuncT(dat, ft_value, weights[ft_idx]);
}

template <class DataT, void (*FuncT)(DataT&, const float, float), class WeightsT>
inline void call_FuncT(DataT& dat, const WeightsT& weights, const float ft_value, const uint64_t ft_idx)
{
  FuncT(dat, ft_value, weights[ft_idx]);
}

template <class DataT, void (*FuncT)(DataT&, float, uint64_t), class WeightsT>
inline void call_FuncT(DataT& dat, WeightsT& /*weights*/, const float ft_value, const uint64_t ft_idx)
{
  FuncT(dat, ft_value, ft_idx);
}

// state data used in non-recursive feature generation algorithm
// contains N feature_gen_data records (where N is length of interaction)
struct feature_gen_data
{
  uint64_t hash = 0;              // hash of feature interactions of previous namespaces in the list
  float x = 1.f;                  // value of feature interactions of previous namespaces in the list
                                  // than once calculated at preprocessing together with same_ns
  bool self_interaction = false;  // namespace interacting with itself
  features::const_audit_iterator begin_it;
  features::const_audit_iterator current_it;
  features::const_audit_iterator end_it;

  feature_gen_data(features::const_audit_iterator begin, features::const_audit_iterator end)
      : begin_it(begin), current_it(begin), end_it(end)
  {
  }
};

inline bool term_is_empty(namespace_index term, const std::array<features, NUM_NAMESPACES>& feature_groups)
{
  return feature_groups[term].empty();
}

inline bool has_empty_interaction_quadratic(
    const std::array<features, NUM_NAMESPACES>& feature_groups, const std::vector<namespace_index>& namespace_indexes)
{
  return term_is_empty(namespace_indexes[0], feature_groups) || term_is_empty(namespace_indexes[1], feature_groups);
}

inline bool has_empty_interaction_cubic(
    const std::array<features, NUM_NAMESPACES>& feature_groups, const std::vector<namespace_index>& namespace_indexes)
{
  return term_is_empty(namespace_indexes[0], feature_groups) || term_is_empty(namespace_indexes[1], feature_groups) ||
      term_is_empty(namespace_indexes[2], feature_groups);
  ;
}

inline bool has_empty_interaction(
    const std::array<features, NUM_NAMESPACES>& feature_groups, const std::vector<namespace_index>& namespace_indexes)
{
  return std::any_of(namespace_indexes.begin(), namespace_indexes.end(),
      [&](namespace_index idx) { return term_is_empty(idx, feature_groups); });
}

// The inline function below may be adjusted to change the way
// synthetic (interaction) features' values are calculated, e.g.,
// fabs(value1-value2) or even value1>value2?1.0:-1.0
// Beware - its result must be non-zero.
inline float INTERACTION_VALUE(float value1, float value2) { return value1 * value2; }

// uncomment line below to disable usage of inner 'for' loops for pair and triple interactions
// end switch to usage of non-recursive feature generation algorithm for interactions of any length

// #define GEN_INTER_LOOP

std::tuple<features_range_t, features_range_t> inline generate_quadratic_char_combination(
    const std::array<features, NUM_NAMESPACES>& feature_groups, namespace_index ns_idx1, namespace_index ns_idx2)
{
  return {std::make_tuple(std::make_pair(feature_groups[ns_idx1].audit_begin(), feature_groups[ns_idx1].audit_end()),
      std::make_pair(feature_groups[ns_idx2].audit_begin(), feature_groups[ns_idx2].audit_end()))};
}

std::tuple<features_range_t, features_range_t, features_range_t> inline generate_cubic_char_combination(
    const std::array<features, NUM_NAMESPACES>& feature_groups, namespace_index ns_idx1, namespace_index ns_idx2,
    namespace_index ns_idx3)
{
  return {std::make_tuple(std::make_pair(feature_groups[ns_idx1].audit_begin(), feature_groups[ns_idx1].audit_end()),
      std::make_pair(feature_groups[ns_idx2].audit_begin(), feature_groups[ns_idx2].audit_end()),
      std::make_pair(feature_groups[ns_idx3].audit_begin(), feature_groups[ns_idx3].audit_end()))};
}

std::vector<features_range_t> inline generate_generic_char_combination(
    const std::array<features, NUM_NAMESPACES>& feature_groups, const std::vector<namespace_index>& namespace_indexes)
{
  std::vector<features_range_t> inter;
  for (const auto namespace_index : namespace_indexes)
  { inter.emplace_back(feature_groups[namespace_index].audit_begin(), feature_groups[namespace_index].audit_end()); }
  return inter;
}

template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT), bool audit,
    void (*audit_func)(DataT&, const audit_strings*), class WeightsT>
void inner_kernel(DataT& dat, features::const_audit_iterator& begin, features::const_audit_iterator& end,
    const uint64_t offset, WeightsT& weights, feature_value ft_value, feature_index halfhash)
{
  if (audit)
  {
    for (; begin != end; ++begin)
    {
      audit_func(dat, begin.audit() == nullptr ? &EMPTY_AUDIT_STRINGS : begin.audit());
      call_FuncT<DataT, FuncT>(
          dat, weights, INTERACTION_VALUE(ft_value, begin.value()), (begin.index() ^ halfhash) + offset);
      audit_func(dat, nullptr);
    }
  }
  else
  {
    for (; begin != end; ++begin)
      call_FuncT<DataT, FuncT>(
          dat, weights, INTERACTION_VALUE(ft_value, begin.value()), (begin.index() ^ halfhash) + offset);
  }
}

template <bool Audit, typename KernelFuncT, typename AuditFuncT>
size_t process_quadratic_interaction(const std::tuple<features_range_t, features_range_t>& range, bool permutations,
    const KernelFuncT& kernel_func, const AuditFuncT& audit_func)
{
  size_t num_features = 0;
  auto first_begin = std::get<0>(range).first;
  const auto& first_end = std::get<0>(range).second;
  const auto& second_begin = std::get<1>(range).first;
  auto& second_end = std::get<1>(range).second;

  const bool same_namespace = (!permutations && (first_begin == second_begin));
  size_t i = 0;
  for (; first_begin != first_end; ++first_begin)
  {
    feature_index halfhash = FNV_prime * first_begin.index();
    if (Audit) { audit_func(first_begin.audit() != nullptr ? first_begin.audit() : &EMPTY_AUDIT_STRINGS); }
    // next index differs for permutations and simple combinations
    auto begin = second_begin;
    if (same_namespace) { begin += i; }
    num_features += std::distance(begin, second_end);
    kernel_func(begin, second_end, first_begin.value(), halfhash);
    if (Audit) audit_func(nullptr);
    i++;
  }
  return num_features;
}

template <bool Audit, typename KernelFuncT, typename AuditFuncT>
size_t process_cubic_interaction(const std::tuple<features_range_t, features_range_t, features_range_t>& range,
    bool permutations, const KernelFuncT& kernel_func, const AuditFuncT& audit_func)
{
  size_t num_features = 0;
  auto first_begin = std::get<0>(range).first;
  const auto& first_end = std::get<0>(range).second;
  const auto& second_begin = std::get<1>(range).first;
  auto second_end = std::get<1>(range).second;
  const auto& third_begin = std::get<2>(range).first;
  auto& third_end = std::get<2>(range).second;

  // don't compare 1 and 3 as interaction is sorted
  const bool same_namespace1 = (!permutations && (first_begin == second_begin));
  const bool same_namespace2 = (!permutations && (second_begin == third_begin));

  size_t i = 0;
  for (; first_begin != first_end; ++first_begin)
  {
    if (Audit) { audit_func(first_begin.audit() != nullptr ? first_begin.audit() : &EMPTY_AUDIT_STRINGS); }

    const uint64_t halfhash1 = FNV_prime * first_begin.index();
    const float first_ft_value = first_begin.value();
    size_t j = 0;
    if (same_namespace1)  // next index differs for permutations and simple combinations
    { j = i; }

    for (auto inner_second_begin = second_begin + j; inner_second_begin != second_end; ++inner_second_begin)
    {
      // f3 x k*(f2 x k*f1)
      if (Audit)
      { audit_func(inner_second_begin.audit() != nullptr ? inner_second_begin.audit() : &EMPTY_AUDIT_STRINGS); }
      feature_index halfhash = FNV_prime * (halfhash1 ^ inner_second_begin.index());
      feature_value ft_value = INTERACTION_VALUE(first_ft_value, inner_second_begin.value());

      auto begin = third_begin;
      // next index differs for permutations and simple combinations
      if (same_namespace2) { begin += j; }
      num_features += std::distance(begin, third_end);
      kernel_func(begin, third_end, ft_value, halfhash);
      if (Audit) audit_func(nullptr);
      j++;
    }  // end for (snd)
    if (Audit) audit_func(nullptr);
    i++;
  }
  return num_features;
}

template <bool Audit, typename KernelFuncT, typename AuditFuncT>
size_t process_generic_interaction(const std::vector<features_range_t>& range, bool permutations,
    const KernelFuncT& kernel_func, const AuditFuncT& audit_func)
{
  size_t num_features = 0;
  std::vector<feature_gen_data> state_data;
  state_data.reserve(range.size());
  // preparing state data
  for (const auto& r : range) { state_data.emplace_back(r.first, r.second); }

  if (!permutations)  // adjust state_data for simple combinations
  {                   // if permutations mode is disabled then namespaces in ns are already sorted and thus grouped
    // (in fact, currently they are sorted even for enabled permutations mode)
    // let's go throw the list and calculate number of features to skip in namespaces which
    // repeated more than once to generate only simple combinations of features

    for (auto fgd = state_data.data() + (state_data.size() - 1); fgd > state_data.data(); --fgd)
    {
      const auto prev = fgd - 1;
      fgd->self_interaction =
          (fgd->current_it == prev->current_it);  // state_data.begin().self_interaction is always false
    }
  }  // end of state_data adjustment

  const auto gen_data_head = state_data.data();                            // always equal to first ns
  const auto gen_data_last = state_data.data() + (state_data.size() - 1);  // always equal to last ns

  feature_gen_data* cur_data = gen_data_head;

  // generic feature generation cycle for interactions of any length
  bool do_it = true;
  while (do_it)
  {
    if (cur_data < gen_data_last)  // can go further through the list of namespaces in interaction
    {
      feature_gen_data* next_data = cur_data + 1;

      if (next_data->self_interaction)
      {  // if next namespace is same, we should start with loop_idx + 1 to avoid feature interaction with itself
        // unless feature has value x and x != x*x. E.g. x != 0 and x != 1. Features with x == 0 are already
        // filtered out in parse_args.cc::maybeFeature().
        const auto current_offset = cur_data->current_it - cur_data->begin_it;
        next_data->current_it = next_data->begin_it;
        next_data->current_it += current_offset;
      }
      else
      {
        next_data->current_it = next_data->begin_it;
      }

      if (Audit) audit_func((*cur_data->current_it).audit());

      if (cur_data == gen_data_head)  // first namespace
      {
        next_data->hash = FNV_prime * (*cur_data->current_it).index();
        next_data->x = (*cur_data->current_it).value();  // data->x == 1.
      }
      else
      {  // feature2 xor (16777619*feature1)
        next_data->hash = FNV_prime * (cur_data->hash ^ (*cur_data->current_it).index());
        next_data->x = INTERACTION_VALUE((*cur_data->current_it).value(), cur_data->x);
      }
      ++cur_data;
    }
    else
    {
      // last namespace - iterate its features and go back
      // start value is not a constant in this case
      size_t start_i = 0;
      if (!permutations) { start_i = gen_data_last->current_it - gen_data_last->begin_it; }

      feature_value ft_value = gen_data_last->x;
      feature_index halfhash = gen_data_last->hash;

      auto begin = cur_data->begin_it + start_i;
      num_features += (cur_data->end_it - begin);
      kernel_func(begin, cur_data->end_it, ft_value, halfhash);
      // trying to go back increasing loop_idx of each namespace by the way
      bool go_further;
      do
      {
        --cur_data;
        ++cur_data->current_it;
        go_further = cur_data->current_it == cur_data->end_it;
        if (Audit) audit_func(nullptr);
      } while (go_further && cur_data != gen_data_head);

      do_it = !(cur_data == gen_data_head && go_further);
      // if do_it==false - we've reached 0 namespace but its 'cur_data.loop_idx > cur_data.loop_end' -> exit the
      // while loop
    }  // if last namespace
  }    // while do_it

  return num_features;
}

// this templated function generates new features for given example and set of interactions
// and passes each of them to given function FuncT()
// it must be in header file to avoid compilation problems
template <class DataT, class WeightOrIndexT, void (*FuncT)(DataT&, float, WeightOrIndexT), bool audit,
    void (*audit_func)(DataT&, const audit_strings*),
    class WeightsT>  // nullptr func can't be used as template param in old compilers
inline void generate_interactions(const std::vector<std::vector<namespace_index>>& interactions, bool permutations,
    example_predict& ec, DataT& dat, WeightsT& weights,
    size_t& num_features)  // default value removed to eliminate ambiguity in old complers
{
  num_features = 0;
  // often used values
  const auto inner_kernel_func = [&](features::const_audit_iterator begin, features::const_audit_iterator end,
                                     feature_value value, feature_index index) {
    inner_kernel<DataT, WeightOrIndexT, FuncT, audit, audit_func>(dat, begin, end, ec.ft_offset, weights, value, index);
  };

  const auto depth_audit_func = [&](const audit_strings* audit_str) { audit_func(dat, audit_str); };

  // current list of namespaces to interact.
  for (const auto& ns : interactions)
  {

#ifndef GEN_INTER_LOOP

    // unless GEN_INTER_LOOP is defined we use nested 'for' loops for interactions length 2 (pairs) and 3 (triples)
    // and generic non-recursive algorithm for all other cases.
    // nested 'for' loops approach is faster, but can't be used for interaction of any length.
    const size_t len = ns.size();
    if (len == 2)  // special case of pairs
    {
      // Skip over any interaction with an empty namespace.
      if (has_empty_interaction_quadratic(ec.feature_space, ns)) { continue; }
      num_features +=
          process_quadratic_interaction<audit>(generate_quadratic_char_combination(ec.feature_space, ns[0], ns[1]),
              permutations, inner_kernel_func, depth_audit_func);
    }
    else if (len == 3)  // special case for triples
    {
      // Skip over any interaction with an empty namespace.
      if (has_empty_interaction_cubic(ec.feature_space, ns)) { continue; }
      num_features +=
          process_cubic_interaction<audit>(generate_cubic_char_combination(ec.feature_space, ns[0], ns[1], ns[2]),
              permutations, inner_kernel_func, depth_audit_func);
    }
    else  // generic case: quatriples, etc.
#endif
    {
      // Skip over any interaction with an empty namespace.
      if (has_empty_interaction(ec.feature_space, ns)) { continue; }
      num_features += process_generic_interaction<audit>(
          generate_generic_char_combination(ec.feature_space, ns), permutations, inner_kernel_func, depth_audit_func);
    }
  }
}  // foreach interaction in all.interactions

}  // namespace INTERACTIONS
