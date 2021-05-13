// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"
#include "example_predict.h"
#include "interactions.h"

#include <vector>
#include <set>

VW::LEARNER::base_learner* generate_interactions_setup(VW::config::options_i& options, vw& all);

std::vector<std::vector<namespace_index>> generate_combinations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);
std::vector<std::vector<namespace_index>> generate_permutations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);

inline bool contains_wildcard(const std::vector<namespace_index>& interaction)
{
  return std::find(interaction.begin(), interaction.end(), wildcard_namespace) != interaction.end();
}

inline bool sort_interactions_comparator(const std::vector<namespace_index>& a, const std::vector<namespace_index>& b)
{
  if (a.size() != b.size()) { return a.size() > b.size(); }
  for (size_t i = 0; i < a.size(); i++)
  {
    if (a[i] < b[i])
      return true;
    else if (a[i] == b[i])
      continue;
    else
      return false;
  }
  return false;
}

inline std::vector<std::vector<namespace_index>> expand_quadratics_wildcard_interactions(
    bool leave_duplicate_interactions, const std::set<namespace_index>& new_example_indices)
{
  std::vector<std::vector<namespace_index>> interactions;

  for (auto it = new_example_indices.begin(); it != new_example_indices.end(); ++it)
  {
    auto idx1 = *it;
    interactions.push_back({idx1, idx1});

    for (auto jt = it; jt != new_example_indices.end(); ++jt)
    {
      auto idx2 = *jt;
      interactions.push_back({idx1, idx2});
      interactions.push_back({idx2, idx2});
      if (leave_duplicate_interactions) { interactions.push_back({idx2, idx1}); }
    }
  }
  return interactions;
}

using generate_func_t = std::vector<std::vector<namespace_index>>(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);

template <generate_func_t generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<namespace_index>> compile_interaction(
    const std::vector<namespace_index>& interaction, const std::set<namespace_index>& indices)
{
  std::vector<size_t> insertion_indices;
  std::vector<namespace_index> insertion_ns;
  size_t num_wildcards = 0;
  for (size_t i = 0; i < interaction.size(); i++)
  {
    if (interaction[i] != wildcard_namespace)
    {
      insertion_indices.push_back(i);
      insertion_ns.push_back(interaction[i]);
    }
    else
    {
      num_wildcards++;
    }
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

template <generate_func_t generate_func, bool leave_duplicate_interactions>
std::vector<std::vector<namespace_index>> compile_interactions(
    const std::vector<std::vector<namespace_index>>& interactions, const std::set<namespace_index>& indices)
{
  std::vector<std::vector<namespace_index>> final_interactions;

  for (const auto& inter : interactions)
  {
    if (contains_wildcard(inter))
    {
      auto compiled = compile_interaction<generate_func, leave_duplicate_interactions>(inter, indices);
      std::copy(compiled.begin(), compiled.end(), std::back_inserter(final_interactions));
    }
    else
    {
      final_interactions.push_back(inter);
    }
  }
  std::sort(final_interactions.begin(), final_interactions.end(), INTERACTIONS::sort_interactions_comparator);
  size_t removed_cnt = 0;
  size_t sorted_cnt = 0;
  INTERACTIONS::sort_and_filter_duplicate_interactions(
      final_interactions, !leave_duplicate_interactions, removed_cnt, sorted_cnt);
  return final_interactions;
}
