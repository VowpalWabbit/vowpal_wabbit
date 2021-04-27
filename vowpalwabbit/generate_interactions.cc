// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "reductions.h"
#include "v_array.h"

#include "io/logger.h"
#include "interactions.h"
#include "vw_math.h"
#include <algorithm>

using namespace VW::config;

namespace logger = VW::io::logger;

struct generate_interactions
{
};

std::vector<namespace_index> indices_to_values_one_based(
    const std::vector<size_t>& indices, const v_array<namespace_index>& values)
{
  std::vector<namespace_index> result;
  result.reserve(indices.size());
  for (size_t i = 0; i < indices.size(); i++) { result.push_back(values[indices[i] - 1]); }
  return result;
}

std::vector<namespace_index> indices_to_values_ignore_last_index(
    const std::vector<size_t>& indices, const v_array<namespace_index>& values)
{
  std::vector<namespace_index> result;
  result.reserve(indices.size() - 1);
  for (size_t i = 0; i < indices.size() - 1; i++) { result.push_back(values[indices[i]]); }
  return result;
}

std::vector<std::vector<namespace_index>> generate_combinations_with_repetiton(
    const v_array<namespace_index>& namespaces, size_t num_to_pick)
{
  std::vector<std::vector<namespace_index>> result;
  result.reserve(VW::math::number_of_combinations_with_repetition(namespaces.size(), num_to_pick));

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
        for (int k = i; k >= 0; --k) { indices[k] = indices[i + 1]; }
      }
    }

    if (indices[num_to_pick] > 0) break;
    result.emplace_back(indices_to_values_ignore_last_index(indices, namespaces));

    indices[0] += 1;
  }

  return result;
}

using generate_func_t = std::vector<std::vector<namespace_index>>(
    const v_array<namespace_index>& namespaces, size_t num_to_pick);

std::vector<std::vector<namespace_index>> generate_permutations_with_repetition(
    const v_array<namespace_index>& namespaces, size_t num_to_pick)
{
  std::vector<std::vector<namespace_index>> result;
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

      result.push_back(indices_to_values_one_based(one_based_chosen_indices, namespaces));
    }
  }

  return result;
}

bool contains_wildcard(const std::vector<namespace_index>& interaction)
{
  return std::find(interaction.begin(), interaction.end(), ':') != interaction.end();
}

template <generate_func_t generate_func>
std::vector<std::vector<namespace_index>> compile_interaction(
    const std::vector<namespace_index>& interaction, const v_array<namespace_index>& indices)
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

template <generate_func_t generate_func>
std::vector<std::vector<namespace_index>> compile_interactions(
    const std::vector<std::vector<namespace_index>>& interactions, const v_array<namespace_index>& indices)
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
  size_t removed_cnt, sorted_cnt;
  INTERACTIONS::sort_and_filter_duplicate_interactions(final_interactions, false, removed_cnt, sorted_cnt);
  return final_interactions;
}

template <bool is_learn, generate_func_t generate_func>
void transform_single_ex(generate_interactions& in, VW::LEARNER::single_learner& base, example& ec)
{
  std::vector<std::vector<namespace_index>> final_interactions =
      compile_interactions<generate_func>(*ec.interactions, ec.indices);
  auto* saved_interactions = ec.interactions;
  ec.interactions = &final_interactions;

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
  ec.interactions = saved_interactions;
}

VW::LEARNER::base_learner* generate_interactions_setup(options_i& options, vw& all)
{
  bool leave_duplicate_interactions;
  option_group_definition new_options("scorer options");
  new_options.add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
                      .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                            "duplicate: '-q ab -q ba' and a lot more in '-q ::'."));
  options.add_and_parse(new_options);

  using learn_pred_func_t = void (*)(generate_interactions&, VW::LEARNER::single_learner&, example&);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;

  if (leave_duplicate_interactions)
  {
    learn_func = transform_single_ex<true, generate_permutations_with_repetition>;
    pred_func = transform_single_ex<false, generate_permutations_with_repetition>;
  }
  else
  {
    learn_func = transform_single_ex<true, generate_combinations_with_repetiton>;
    pred_func = transform_single_ex<false, generate_combinations_with_repetiton>;
  }

  auto data = VW::make_unique<generate_interactions>();
  auto* base = as_singleline(setup_base(options, all));
  auto* l = VW::LEARNER::make_reduction_learner(
      std::move(data), base, learn_func, pred_func, all.get_setupfn_name(generate_interactions_setup))
                .build();
  return VW::LEARNER::make_base(*l);
}
