// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "generate_interactions.h"
#include "reductions.h"
#include "v_array.h"

#include "io/logger.h"
#include "interactions.h"
#include "vw_math.h"
#include <algorithm>
#include <iterator>

using namespace VW::config;

namespace logger = VW::io::logger;

struct generate_interactions
{
  std::set<namespace_index> all_seen_namespaces;
  std::vector<std::vector<namespace_index>> generated_interactions;
};

std::vector<namespace_index> indices_to_values_one_based(
    const std::vector<size_t>& indices, const std::set<namespace_index>& values)
{
  std::vector<namespace_index> result;
  result.reserve(indices.size());
  for (size_t i = 0; i < indices.size(); i++)
  {
    auto it = values.begin();
    std::advance(it, indices[i] - 1);
    result.push_back(*it);
  }
  return result;
}

std::vector<namespace_index> indices_to_values_ignore_last_index(
    const std::vector<size_t>& indices, const std::set<namespace_index>& values)
{
  std::vector<namespace_index> result;
  result.reserve(indices.size() - 1);
  for (size_t i = 0; i < indices.size() - 1; i++)
  {
    auto it = values.begin();
    std::advance(it, indices[i]);
    result.push_back(*it);
  }
  return result;
}

std::vector<std::vector<namespace_index>> generate_combinations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick)
{
  std::vector<std::vector<namespace_index>> result;
  // This computation involves factorials and so can only be done with relatively small inputs.
  if ((namespaces.size() + num_to_pick) <= 21)
  { result.reserve(VW::math::number_of_combinations_with_repetition(namespaces.size(), num_to_pick)); }

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

std::vector<std::vector<namespace_index>> generate_permutations_with_repetition(
    const std::set<namespace_index>& namespaces, size_t num_to_pick)
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

template <generate_func_t generate_func, bool leave_duplicate_interactions>
void update_interactions_if_new_namespace_seen(generate_interactions& data,
    const std::vector<std::vector<namespace_index>>& interactions, const v_array<namespace_index>& new_example_indices)
{
  auto prev_count = data.all_seen_namespaces.size();
  data.all_seen_namespaces.insert(new_example_indices.begin(), new_example_indices.end());

  if (prev_count != data.all_seen_namespaces.size())
  {
    // Generating interactions for the constant namespace doesn't really make sense since it will essentially just be
    // another copy of each feature itself. To prevent these getting generated we remove it from the set temporarily
    // then add it back after.
    auto constant_namespace_it = data.all_seen_namespaces.find(constant_namespace);
    if (constant_namespace_it != data.all_seen_namespaces.end())
    { data.all_seen_namespaces.erase(constant_namespace_it); }
    data.generated_interactions =
        compile_interactions<generate_func, leave_duplicate_interactions>(interactions, data.all_seen_namespaces);
    data.all_seen_namespaces.insert(constant_namespace);
  }
}

template <bool is_learn, generate_func_t generate_func, bool leave_duplicate_interactions>
void transform_single_ex(generate_interactions& data, VW::LEARNER::single_learner& base, example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      data, *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
  ec.interactions = saved_interactions;
}

template <generate_func_t generate_func, bool leave_duplicate_interactions>
void update(generate_interactions& data, VW::LEARNER::single_learner& base, example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      data, *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;
  base.update(ec);
  ec.interactions = saved_interactions;
}

template <generate_func_t generate_func, bool leave_duplicate_interactions>
inline void multipredict(generate_interactions& data, VW::LEARNER::single_learner& base, example& ec, size_t count,
    size_t, polyprediction* pred, bool finalize_predictions)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      data, *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;
  base.multipredict(ec, 0, count, pred, finalize_predictions);
  ec.interactions = saved_interactions;
}

VW::LEARNER::base_learner* generate_interactions_setup(options_i& options, vw& all)
{
  bool leave_duplicate_interactions;
  option_group_definition new_options("Generate interactions");
  new_options.add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
                      .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                            "duplicate: '-q ab -q ba' and a lot more in '-q ::'."));
  options.add_and_parse(new_options);

  auto interactions_spec_contains_wildcards = false;
  for (const auto& inter : all.interactions)
  {
    if (contains_wildcard(inter))
    {
      interactions_spec_contains_wildcards = true;
      break;
    }
  }

  // If there are no wildcards, then no expansion is required.
  // ccb_explore_adf adds a wildcard post setup and so this reduction must be turned on.
  if (!interactions_spec_contains_wildcards && !options.was_supplied("ccb_explore_adf")) return nullptr;

  using learn_pred_func_t = void (*)(generate_interactions&, VW::LEARNER::single_learner&, example&);
  using multipredict_func_t =
      void (*)(generate_interactions&, VW::LEARNER::single_learner&, example&, size_t, size_t, polyprediction*, bool);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  learn_pred_func_t update_func;
  multipredict_func_t multipredict_func;

  if (leave_duplicate_interactions)
  {
    learn_func = transform_single_ex<true, generate_permutations_with_repetition, true>;
    pred_func = transform_single_ex<false, generate_permutations_with_repetition, true>;
    update_func = update<generate_permutations_with_repetition, true>;
    multipredict_func = multipredict<generate_permutations_with_repetition, true>;
  }
  else
  {
    learn_func = transform_single_ex<true, generate_combinations_with_repetition, false>;
    pred_func = transform_single_ex<false, generate_combinations_with_repetition, false>;
    update_func = update<generate_combinations_with_repetition, false>;
    multipredict_func = multipredict<generate_combinations_with_repetition, false>;
  }

  auto data = VW::make_unique<generate_interactions>();
  auto* base = as_singleline(setup_base(options, all));
  auto* l = VW::LEARNER::make_reduction_learner(
      std::move(data), base, learn_func, pred_func, all.get_setupfn_name(generate_interactions_setup))
                .set_learn_returns_prediction(base->learn_returns_prediction)
                .set_update(update_func)
                .set_multipredict(multipredict_func)
                .build();
  return VW::LEARNER::make_base(*l);
}
