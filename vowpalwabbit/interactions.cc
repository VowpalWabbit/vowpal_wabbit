// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "interactions.h"

#include "vw_exception.h"
#include "vw_math.h"
#include "v_array.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include <sstream>
#include <cfloat>
#include <algorithm>
#include <iterator>

using namespace VW::config;

namespace INTERACTIONS
{
namespace
{
template <typename FuncT>
void for_each_value(const std::array<features, NUM_NAMESPACES>& feature_spaces, namespace_index term, const FuncT& func)
{
  for (auto value : feature_spaces[term].values) { func(value); }
}

template <typename FuncT>
void for_each_value(const std::array<features, NUM_NAMESPACES>& feature_spaces, extent_term term, const FuncT& func)
{
  feature_spaces[term.first].foreach_feature_for_hash(
      term.second, [&](features::const_audit_iterator it) { func(it.value()); });
}

float calc_sum_ft_squared_for_term(const std::array<features, NUM_NAMESPACES>& feature_spaces, extent_term term)
{
  float sum_feat_sq_in_inter = 0.f;
  feature_spaces[term.first].foreach_feature_for_hash(
      term.second, [&](features::const_audit_iterator it) { sum_feat_sq_in_inter += it.value() * it.value(); });
  return sum_feat_sq_in_inter;
}

float calc_sum_ft_squared_for_term(const std::array<features, NUM_NAMESPACES>& feature_spaces, namespace_index term)
{
  return feature_spaces[term].sum_feat_sq;
}

template <typename InteractionTermT>
float calculate_count_and_sum_ft_sq_for_permutations(const std::array<features, NUM_NAMESPACES>& feature_spaces,
    const std::vector<std::vector<InteractionTermT>>& interactions)
{
  float sum_feat_sq_in_inter_outer = 0.;
  for (const auto& interaction : interactions)
  {
    float sum_feat_sq_in_inter = 1.;
    for (auto term : interaction) { sum_feat_sq_in_inter *= calc_sum_ft_squared_for_term(feature_spaces, term); }
    sum_feat_sq_in_inter_outer += sum_feat_sq_in_inter;
  }

  return sum_feat_sq_in_inter_outer;
}

template <typename InteractionTermT>
float calculate_count_and_sum_ft_sq_for_combinations(const std::array<features, NUM_NAMESPACES>& feature_spaces,
    const std::vector<std::vector<InteractionTermT>>& interactions)
{
  std::vector<float> results;

  float sum_feat_sq_in_inter_outer = 0.;
  for (const auto& inter : interactions)
  {
    float sum_feat_sq_in_inter = 1.;

    for (auto interaction_term_it = inter.begin(); interaction_term_it != inter.end(); ++interaction_term_it)
    {
      if ((interaction_term_it == inter.end() - 1) ||
          (*interaction_term_it != *(interaction_term_it + 1)))  // neighbour namespaces are different
      {
        sum_feat_sq_in_inter *= calc_sum_ft_squared_for_term(feature_spaces, *interaction_term_it);
        if (sum_feat_sq_in_inter == 0.f) { break; }
      }
      else
      {
        // already compared ns == ns+1
        size_t order_of_inter = 2;
        for (auto ns_end = interaction_term_it + 2; ns_end < inter.end(); ++ns_end)
        {
          if (*interaction_term_it == *ns_end) { ++order_of_inter; }
          else
          {
            // Since the list is sorted, as soon as they don't match we can exit.
            break;
          }
        }
        results.resize(order_of_inter);
        std::fill(results.begin(), results.end(), 0.f);

        for_each_value(feature_spaces, *interaction_term_it, [&](float value) {
          const float x = value * value;
          results[0] += x;
          for (size_t j = 1; j < order_of_inter; ++j) { results[j] += results[j - 1] * x; }
        });

        sum_feat_sq_in_inter *= results[order_of_inter - 1];
        if (sum_feat_sq_in_inter == 0.f) { break; }

        interaction_term_it += order_of_inter - 1;  // jump over whole block
      }
    }

    sum_feat_sq_in_inter_outer += sum_feat_sq_in_inter;
  }

  return sum_feat_sq_in_inter_outer;
}
}  // namespace

// returns number of new features that will be generated for example and sum of their squared values
float eval_sum_ft_squared_of_generated_ft(bool permutations,
    const std::vector<std::vector<namespace_index>>& interactions,
    const std::vector<std::vector<extent_term>>& extent_interactions,
    const std::array<features, NUM_NAMESPACES>& feature_spaces)
{
  float sum_ft_sq = 0.f;

  if (permutations)
  {
    sum_ft_sq += calculate_count_and_sum_ft_sq_for_permutations(feature_spaces, interactions);
    sum_ft_sq += calculate_count_and_sum_ft_sq_for_permutations(feature_spaces, extent_interactions);
  }
  else
  {
    sum_ft_sq += calculate_count_and_sum_ft_sq_for_combinations(feature_spaces, interactions);
    sum_ft_sq += calculate_count_and_sum_ft_sq_for_combinations(feature_spaces, extent_interactions);
  }
  return sum_ft_sq;
}

bool sort_interactions_comparator(const std::vector<namespace_index>& a, const std::vector<namespace_index>& b)
{
  if (a.size() != b.size()) { return a.size() < b.size(); }
  return a < b;
}

/*
 *   Sorting and filtering duplicate interactions
 */

// returns true if iteraction contains one or more duplicated namespaces
// with one exeption - returns false if interaction made of one namespace
// like 'aaa' as it has no sense to sort such things.

inline bool must_be_left_sorted(const std::vector<namespace_index>& oi)
{
  if (oi.size() <= 1) return true;  // one letter in std::string - no need to sort

  bool diff_ns_found = false;
  bool pair_found = false;

  for (auto i = std::begin(oi); i != std::end(oi) - 1; ++i)
    if (*i == *(i + 1))  // pair found
    {
      if (diff_ns_found) return true;  // case 'abb'
      pair_found = true;
    }
    else
    {
      if (pair_found) return true;  // case 'aab'
      diff_ns_found = true;
    }

  return false;  // 'aaa' or 'abc'
}

std::vector<std::vector<namespace_index>> expand_quadratics_wildcard_interactions(
    bool leave_duplicate_interactions, const std::set<namespace_index>& new_example_indices)
{
  std::set<std::vector<namespace_index>> interactions;

  for (auto it = new_example_indices.begin(); it != new_example_indices.end(); ++it)
  {
    auto idx1 = *it;
    interactions.insert({idx1, idx1});

    for (auto jt = it; jt != new_example_indices.end(); ++jt)
    {
      auto idx2 = *jt;
      interactions.insert({idx1, idx2});
      interactions.insert({idx2, idx2});
      if (leave_duplicate_interactions) { interactions.insert({idx2, idx1}); }
    }
  }
  return std::vector<std::vector<namespace_index>>(interactions.begin(), interactions.end());
}

// used from parse_args.cc
// filter duplicate namespaces treating them as unordered sets of namespaces.
// also sort namespaces in interactions containing duplicate namespaces to make sure they are grouped together.

void sort_and_filter_duplicate_interactions(
    std::vector<std::vector<namespace_index>>& vec, bool filter_duplicates, size_t& removed_cnt, size_t& sorted_cnt)
{
  // 2 out parameters
  removed_cnt = 0;
  sorted_cnt = 0;

  // interaction value sort + original position
  std::vector<std::pair<std::vector<namespace_index>, size_t>> vec_sorted;
  for (size_t i = 0; i < vec.size(); ++i)
  {
    std::vector<namespace_index> sorted_i(vec[i]);
    std::stable_sort(std::begin(sorted_i), std::end(sorted_i));
    vec_sorted.push_back(std::make_pair(sorted_i, i));
  }

  if (filter_duplicates)
  {
    // remove duplicates
    std::stable_sort(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<namespace_index>, size_t> const& a,
            std::pair<std::vector<namespace_index>, size_t> const& b) { return a.first < b.first; });
    auto last = unique(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<namespace_index>, size_t> const& a,
            std::pair<std::vector<namespace_index>, size_t> const& b) { return a.first == b.first; });
    vec_sorted.erase(last, vec_sorted.end());

    // report number of removed interactions
    removed_cnt = vec.size() - vec_sorted.size();

    // restore original order
    std::stable_sort(vec_sorted.begin(), vec_sorted.end(),
        [](std::pair<std::vector<namespace_index>, size_t> const& a,
            std::pair<std::vector<namespace_index>, size_t> const& b) { return a.second < b.second; });
  }

  // we have original vector and vector with duplicates removed + corresponding indexes in original vector
  // plus second vector's data is sorted. We can reuse it if we need interaction to be left sorted.
  // let's make a new vector from these two sources - without dulicates and with sorted data whenever it's needed.
  std::vector<std::vector<namespace_index>> res;
  for (auto& i : vec_sorted)
  {
    if (must_be_left_sorted(i.first))
    {
      // if so - copy sorted data to result
      res.push_back(i.first);
      ++sorted_cnt;
    }
    else  // else - move unsorted data to result
      res.push_back(vec[i.second]);
  }

  vec = res;
}

}  // namespace INTERACTIONS
