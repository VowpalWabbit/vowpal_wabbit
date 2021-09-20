// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "example_predict.h"
#include "reductions_fwd.h"
#include "constant.h"

#include <cstddef>

#include <vector>
#include <set>
#include <algorithm>

namespace INTERACTIONS
{
VW::LEARNER::base_learner* generate_interactions_setup(VW::config::options_i& options, vw& all);

constexpr unsigned char printable_start = ' ';
constexpr unsigned char printable_end = '~';
constexpr unsigned char printable_ns_size = printable_end - printable_start;
constexpr uint64_t valid_ns_size =
    printable_end - printable_start - 1;  // -1 to skip characters ':' and '|' excluded in is_valid_ns()

inline constexpr bool is_printable_namespace(const unsigned char ns)
{
  return ns >= printable_start && ns <= printable_end;
}

inline bool contains_wildcard(const std::vector<namespace_index>& interaction)
{
  return std::find(interaction.begin(), interaction.end(), wildcard_namespace) != interaction.end();
}
// function estimates how many new features will be generated for example and their sum(value^2).
float eval_sum_ft_squared_of_generated_ft(bool permutations,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions,
    const std::array<features, NUM_NAMESPACES>& feature_spaces);

std::vector<std::vector<namespace_index>> generate_namespace_combinations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);
std::vector<std::vector<namespace_index>> generate_namespace_permutations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);
using generate_func_t = std::vector<std::vector<namespace_index>>(
    const std::set<namespace_index>& namespaces, size_t num_to_pick);

std::vector<std::vector<namespace_index>> expand_quadratics_wildcard_interactions(
    bool leave_duplicate_interactions, const std::set<namespace_index>& new_example_indices);

bool sort_interactions_comparator(const std::vector<namespace_index>& a, const std::vector<namespace_index>& b);

void sort_and_filter_duplicate_interactions(
    std::vector<std::vector<namespace_index>>& vec, bool filter_duplicates, size_t& removed_cnt, size_t& sorted_cnt);

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

// Compiling an interaction means to expand out wildcards (:) for each index present
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

struct interactions_generator
{
private:
  std::set<namespace_index> all_seen_namespaces;

public:
  std::vector<std::vector<namespace_index>> generated_interactions;

  template <generate_func_t generate_func, bool leave_duplicate_interactions>
  void update_interactions_if_new_namespace_seen(const std::vector<std::vector<namespace_index>>& interactions,
      const v_array<namespace_index>& new_example_indices)
  {
    auto prev_count = all_seen_namespaces.size();
    all_seen_namespaces.insert(new_example_indices.begin(), new_example_indices.end());

    if (prev_count != all_seen_namespaces.size())
    {
      // We do not generate interactions for non-printable namespaces as
      // generally they are used for implementation details and special behavior
      // and not user inputted features.
      std::set<namespace_index> indices_to_interact;
      for (auto ns_index : all_seen_namespaces)
      {
        if (is_printable_namespace(ns_index)) { indices_to_interact.insert(ns_index); }
      }
      generated_interactions.clear();
      if (indices_to_interact.size() > 0)
      {
        generated_interactions =
            compile_interactions<generate_func, leave_duplicate_interactions>(interactions, indices_to_interact);
      }
    }
  }
};

}  // namespace INTERACTIONS
