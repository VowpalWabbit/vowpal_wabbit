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
  return std::find(interaction.begin(), interaction.end(), ':') != interaction.end();
}

using generate_func_t = std::vector<std::vector<namespace_index>>(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);

template <generate_func_t generate_func>
std::vector<std::vector<namespace_index>> compile_interaction(
    const std::vector<namespace_index>& interaction, const std::set<namespace_index>& indices)
{
  std::vector<size_t> insertion_indices;
  std::vector<namespace_index> insertion_ns;
  size_t num_wildcards = 0;
  for (size_t i = 0; i < interaction.size(); i++)
  {
    if (interaction[i] != ':')
    {
      insertion_indices.push_back(i);
      insertion_ns.push_back(interaction[i]);
    }
    else
    {
      num_wildcards++;
    }
  }

  auto result = generate_func(indices, num_wildcards);
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
      auto compiled = compile_interaction<generate_func>(inter, indices);
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
      final_interactions, leave_duplicate_interactions, removed_cnt, sorted_cnt);
  return final_interactions;
}