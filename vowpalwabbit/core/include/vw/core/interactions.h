// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/constant.h"
#include "vw/core/example_predict.h"
#include "vw/core/feature_group.h"
#include "vw/core/vw_fwd.h"
#include "vw/core/vw_math.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <set>
#include <vector>

namespace VW
{
namespace details
{

constexpr unsigned char INTERACTION_NS_START = ' ';
constexpr unsigned char INTERACTION_NS_END = '~';
}  // namespace details

inline constexpr bool is_interaction_ns(const unsigned char ns)
{
  return (ns >= details::INTERACTION_NS_START && ns <= details::INTERACTION_NS_END) ||
      (ns == VW::details::CCB_SLOT_NAMESPACE);
}

inline bool contains_wildcard(const std::vector<VW::namespace_index>& interaction)
{
  return std::find(interaction.begin(), interaction.end(), VW::details::WILDCARD_NAMESPACE) != interaction.end();
}

inline bool contains_wildcard(const std::vector<VW::extent_term>& interaction)
{
  return std::find(interaction.begin(), interaction.end(),
             VW::extent_term{VW::details::WILDCARD_NAMESPACE, VW::details::WILDCARD_NAMESPACE}) != interaction.end();
}

// function estimates how many new features will be generated for example and their sum(value^2).
float eval_sum_ft_squared_of_generated_ft(bool permutations,
    const std::vector<std::vector<VW::namespace_index>>& interactions,
    const std::vector<std::vector<VW::extent_term>>& extent_interactions,
    const std::array<VW::features, VW::NUM_NAMESPACES>& feature_spaces);

template <typename T>
using generate_func_t = std::vector<std::vector<T>>(const std::set<T>& namespaces, size_t num_to_pick);

namespace details
{

template <typename T>
std::vector<T> indices_to_values_one_based(const std::vector<size_t>& indices, const std::set<T>& values)
{
  std::vector<T> result;
  result.reserve(indices.size());
  for (auto idx : indices)
  {
    auto it = values.begin();
    std::advance(it, idx - 1);
    result.push_back(*it);
  }
  return result;
}

template <typename T>
std::vector<T> indices_to_values_ignore_last_index(const std::vector<size_t>& indices, const std::set<T>& values)
{
  std::vector<T> result;
  result.reserve(indices.size() - 1);
  for (size_t i = 0; i < indices.size() - 1; i++)
  {
    auto it = values.begin();
    std::advance(it, indices[i]);
    result.push_back(*it);
  }
  return result;
}

// returns true if iteraction contains one or more duplicated namespaces
// with one exeption - returns false if interaction made of one namespace
// like 'aaa' as it has no sense to sort such things.
template <typename T>
inline bool must_be_left_sorted(const std::vector<T>& oi)
{
  if (oi.size() <= 1)
  {
    return true;  // one letter in std::string - no need to sort
  }

  bool diff_ns_found = false;
  bool pair_found = false;

  for (auto i = std::begin(oi); i != std::end(oi) - 1; ++i)
  {
    if (*i == *(i + 1))  // pair found
    {
      if (diff_ns_found)
      {
        return true;  // case 'abb'
      }
      pair_found = true;
    }
    else
    {
      if (pair_found)
      {
        return true;  // case 'aab'
      }
      diff_ns_found = true;
    }
  }

  return false;  // 'aaa' or 'abc'
}

// used from parse_args.cc
// filter duplicate namespaces treating them as unordered sets of namespaces.
// also sort namespaces in interactions containing duplicate namespaces to make sure they are grouped together.
template <typename T>
void sort_and_filter_duplicate_interactions(
    std::vector<std::vector<T>>& vec, bool filter_duplicates, size_t& removed_cnt, size_t& sorted_cnt)
{
  // 2 out parameters
  removed_cnt = 0;
  sorted_cnt = 0;

  // interaction value sort + original position
  std::vector<std::pair<std::vector<T>, size_t>> vec_sorted;
  for (size_t i = 0; i < vec.size(); ++i)
  {
    std::vector<T> sorted_i(vec[i]);
    std::stable_sort(std::begin(sorted_i), std::end(sorted_i));
    vec_sorted.push_back(std::make_pair(sorted_i, i));
  }

  if (filter_duplicates)
  {
    // remove duplicates
    std::stable_sort(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<T>, size_t> const& a, std::pair<std::vector<T>, size_t> const& b)
        { return a.first < b.first; });
    auto last = unique(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<T>, size_t> const& a, std::pair<std::vector<T>, size_t> const& b)
        { return a.first == b.first; });
    vec_sorted.erase(last, vec_sorted.end());

    // report number of removed interactions
    removed_cnt = vec.size() - vec_sorted.size();

    // restore original order
    std::stable_sort(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<T>, size_t> const& a, std::pair<std::vector<T>, size_t> const& b)
        { return a.second < b.second; });
  }

  // we have original vector and vector with duplicates removed + corresponding indexes in original vector
  // plus second vector's data is sorted. We can reuse it if we need interaction to be left sorted.
  // let's make a new vector from these two sources - without dulicates and with sorted data whenever it's needed.
  std::vector<std::vector<T>> res;
  for (auto& i : vec_sorted)
  {
    if (details::must_be_left_sorted(i.first))
    {
      // if so - copy sorted data to result
      res.push_back(i.first);
      ++sorted_cnt;
    }
    else
    {
      // else - move unsorted data to result
      res.push_back(vec[i.second]);
    }
  }

  vec = res;
}

template <typename T>
std::vector<std::vector<T>> generate_namespace_combinations_with_repetition(
    const std::set<T>& namespaces, size_t num_to_pick)
{
  std::vector<std::vector<T>> result;
  // This computation involves factorials and so can only be done with relatively small inputs.
  // Factorial 22 would result in 64 bit overflow.
  if ((namespaces.size() + num_to_pick) <= 21)
  {
    auto num_combinations = static_cast<uint64_t>(VW::math::number_of_combinations_with_repetition(
        static_cast<uint64_t>(namespaces.size()), static_cast<uint64_t>(num_to_pick)));
    // If this is too large for size_t thats fine we just wont reserve.
    if (static_cast<uint64_t>(num_combinations) < static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
    {
      result.reserve(static_cast<size_t>(num_combinations));
    }
  }

  auto last_index = namespaces.size() - 1;
  // last index is used to signal when done
  std::vector<size_t> indices(num_to_pick + 1, 0);
  while (true)
  {
    for (size_t i = 0; i < num_to_pick; ++i)
    {
      if (indices[i] > last_index)
      {
        // Increment the next index
        indices[i + 1] += 1;
        // Decrement all past indices
        for (int k = static_cast<int>(i); k >= 0; --k) { indices[static_cast<size_t>(k)] = indices[i + 1]; }
      }
    }

    if (indices[num_to_pick] > 0) { break; }
    result.emplace_back(indices_to_values_ignore_last_index(indices, namespaces));

    indices[0] += 1;
  }

  return result;
}

template <typename T>
std::vector<std::vector<T>> generate_namespace_permutations_with_repetition(
    const std::set<T>& namespaces, size_t num_to_pick)
{
  std::vector<std::vector<T>> result;
  result.reserve(VW::math::number_of_permutations_with_repetition(namespaces.size(), num_to_pick));

  std::vector<size_t> one_based_chosen_indices(num_to_pick, 0);
  for (size_t i = 0; i < num_to_pick - 1; i++) { one_based_chosen_indices[i] = 1; }
  one_based_chosen_indices[num_to_pick - 1] = 0;

  size_t number_of_namespaces = namespaces.size();
  size_t next_index = num_to_pick;

  while (true)
  {
    if (one_based_chosen_indices[next_index - 1] == number_of_namespaces)
    {
      next_index--;
      if (next_index == 0) { break; }
    }
    else
    {
      one_based_chosen_indices[next_index - 1]++;
      while (next_index < num_to_pick)
      {
        next_index++;
        one_based_chosen_indices[next_index - 1] = 1;
      }

      result.emplace_back(indices_to_values_one_based(one_based_chosen_indices, namespaces));
    }
  }

  return result;
}

std::vector<std::vector<VW::namespace_index>> expand_quadratics_wildcard_interactions(
    bool leave_duplicate_interactions, const std::set<VW::namespace_index>& new_example_indices);

bool sort_interactions_comparator(const std::vector<VW::namespace_index>& a, const std::vector<VW::namespace_index>& b);

template <generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<VW::namespace_index>> compile_interaction(
    const std::vector<VW::namespace_index>& interaction, const std::set<VW::namespace_index>& indices)
{
  std::vector<size_t> insertion_indices;
  std::vector<VW::namespace_index> insertion_ns;
  size_t num_wildcards = 0;
  for (size_t i = 0; i < interaction.size(); i++)
  {
    if (interaction[i] != VW::details::WILDCARD_NAMESPACE)
    {
      insertion_indices.push_back(i);
      insertion_ns.push_back(interaction[i]);
    }
    else { num_wildcards++; }
  }

  // Quadratic fast path or generic generation function.
  auto result = num_wildcards == 2 ? expand_quadratics_wildcard_interactions(leave_duplicate_interactions, indices)
                                   : generate_func(indices, num_wildcards);
  for (size_t i = 0; i < insertion_indices.size(); i++)
  {
    for (auto& res : result) { res.insert(res.begin() + insertion_indices[i], insertion_ns[i]); }
  }
  return result;
}

template <generate_func_t<VW::extent_term> generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<VW::extent_term>> compile_extent_interaction(
    const std::vector<VW::extent_term>& interaction, const std::set<VW::extent_term>& _all_seen_extents)
{
  std::vector<size_t> insertion_indices;
  std::vector<VW::extent_term> insertion_ns;
  size_t num_wildcards = 0;
  for (size_t i = 0; i < interaction.size(); i++)
  {
    if (interaction[i].first != VW::details::WILDCARD_NAMESPACE)
    {
      insertion_indices.push_back(i);
      insertion_ns.push_back(interaction[i]);
    }
    else { num_wildcards++; }
  }

  auto result = generate_func(_all_seen_extents, num_wildcards);
  for (size_t i = 0; i < insertion_indices.size(); i++)
  {
    for (auto& res : result) { res.insert(res.begin() + insertion_indices[i], insertion_ns[i]); }
  }
  return result;
}

// Compiling an interaction means to expand out wildcards (:) for each index present
template <generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<VW::namespace_index>> compile_interactions(
    const std::vector<std::vector<VW::namespace_index>>& interactions, const std::set<VW::namespace_index>& indices)
{
  std::vector<std::vector<VW::namespace_index>> final_interactions;

  for (const auto& inter : interactions)
  {
    if (contains_wildcard(inter))
    {
      auto compiled = compile_interaction<generate_func, leave_duplicate_interactions>(inter, indices);
      std::copy(compiled.begin(), compiled.end(), std::back_inserter(final_interactions));
    }
    else { final_interactions.push_back(inter); }
  }
  std::sort(final_interactions.begin(), final_interactions.end(), VW::details::sort_interactions_comparator);
  size_t removed_cnt = 0;
  size_t sorted_cnt = 0;
  VW::details::sort_and_filter_duplicate_interactions(
      final_interactions, !leave_duplicate_interactions, removed_cnt, sorted_cnt);
  return final_interactions;
}

template <generate_func_t<VW::extent_term> generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<VW::extent_term>> compile_extent_interactions(
    const std::vector<std::vector<VW::extent_term>>& interactions, const std::set<VW::extent_term>& indices)
{
  std::vector<std::vector<VW::extent_term>> final_interactions;

  for (const auto& inter : interactions)
  {
    if (contains_wildcard(inter))
    {
      auto compiled = compile_extent_interaction<generate_func, leave_duplicate_interactions>(inter, indices);
      std::copy(compiled.begin(), compiled.end(), std::back_inserter(final_interactions));
    }
    else { final_interactions.push_back(inter); }
  }
  size_t removed_cnt = 0;
  size_t sorted_cnt = 0;
  VW::details::sort_and_filter_duplicate_interactions(
      final_interactions, !leave_duplicate_interactions, removed_cnt, sorted_cnt);
  return final_interactions;
}
}  // namespace details

class interactions_generator
{
public:
  std::vector<std::vector<VW::namespace_index>> generated_interactions;
  std::vector<std::vector<VW::extent_term>> generated_extent_interactions;
  bool store_in_reduction_features = false;

  template <generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
  void update_interactions_if_new_namespace_seen(const std::vector<std::vector<VW::namespace_index>>& interactions,
      const VW::v_array<VW::namespace_index>& new_example_indices)
  {
    auto prev_count = _all_seen_namespaces.size();
    _all_seen_namespaces.insert(new_example_indices.begin(), new_example_indices.end());

    if (prev_count != _all_seen_namespaces.size())
    {
      // We do not generate interactions for reserved namespaces as
      // generally they are used for implementation details and special behavior
      // and not user inputted features. The two exceptions are default_namespace
      // and ccb_slot_namespace (the default namespace for CCB slots)
      std::set<VW::namespace_index> indices_to_interact;
      for (auto ns_index : _all_seen_namespaces)
      {
        if (is_interaction_ns(ns_index)) { indices_to_interact.insert(ns_index); }
      }
      generated_interactions.clear();
      if (indices_to_interact.size() > 0)
      {
        generated_interactions = details::compile_interactions<generate_func, leave_duplicate_interactions>(
            interactions, indices_to_interact);
      }
    }
  }

  template <generate_func_t<VW::extent_term> generate_func, bool leave_duplicate_interactions>
  void update_extent_interactions_if_new_namespace_seen(const std::vector<std::vector<VW::extent_term>>& interactions,
      const VW::v_array<VW::namespace_index>& indices,
      const std::array<VW::features, VW::NUM_NAMESPACES>& feature_space)
  {
    auto prev_count = _all_seen_extents.size();
    for (auto ns_index : indices)
    {
      for (const auto& extent : feature_space[ns_index].namespace_extents)
      {
        // Interactions should not be generated for reserved namespaces such as
        // constant. These reserved namespaces use their hash as the namespace
        // character value so we can check if the value is in this range. There
        // is a chance of collisions here though. 0 is a special case as the
        // default case is mapped to a hash of 0 even though it is in index ' ',
        // 32
        if (extent.hash == 0 || extent.hash >= std::numeric_limits<unsigned char>::max() ||
            (extent.hash < std::numeric_limits<unsigned char>::max() &&
                is_interaction_ns(static_cast<unsigned char>(extent.hash))))
        {
          _all_seen_extents.insert({ns_index, extent.hash});
        }
      }
    }

    if (prev_count != _all_seen_extents.size())
    {
      generated_interactions.clear();
      if (!_all_seen_extents.empty())
      {
        generated_extent_interactions =
            details::compile_extent_interactions<generate_func, leave_duplicate_interactions>(
                interactions, _all_seen_extents);
      }
    }
  }

private:
  std::set<VW::namespace_index> _all_seen_namespaces;
  std::set<VW::extent_term> _all_seen_extents;
};

}  // namespace VW

namespace INTERACTIONS  // NOLINT
{
VW_DEPRECATED("Moved to VW namespace")
inline constexpr bool is_interaction_ns(const unsigned char ns) { return VW::is_interaction_ns(ns); }

VW_DEPRECATED("Moved to VW namespace")
inline bool contains_wildcard(const std::vector<VW::namespace_index>& interaction)
{
  return VW::contains_wildcard(interaction);
}

VW_DEPRECATED("Moved to VW namespace")
inline bool contains_wildcard(const std::vector<VW::extent_term>& interaction)
{
  return VW::contains_wildcard(interaction);
}

VW_DEPRECATED("Moved to VW namespace")
inline float eval_sum_ft_squared_of_generated_ft(bool permutations,
    const std::vector<std::vector<VW::namespace_index>>& interactions,
    const std::vector<std::vector<VW::extent_term>>& extent_interactions,
    const std::array<VW::features, VW::NUM_NAMESPACES>& feature_spaces)
{
  return VW::eval_sum_ft_squared_of_generated_ft(permutations, interactions, extent_interactions, feature_spaces);
}

template <typename T>
VW_DEPRECATED("Moved to VW namespace")
void sort_and_filter_duplicate_interactions(
    std::vector<std::vector<T>>& vec, bool filter_duplicates, size_t& removed_cnt, size_t& sorted_cnt)
{
  VW::details::sort_and_filter_duplicate_interactions(vec, filter_duplicates, removed_cnt, sorted_cnt);
}
}  // namespace INTERACTIONS
