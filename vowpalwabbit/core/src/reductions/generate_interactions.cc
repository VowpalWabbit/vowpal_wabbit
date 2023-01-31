// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/generate_interactions.h"

#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/interactions.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_math.h"
#include "vw/io/logger.h"

#include <algorithm>
#include <cfloat>
#include <iterator>
#include <sstream>

using namespace VW::config;

namespace
{
template <bool is_learn, VW::generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
void transform_single_ex(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
  }

  if (is_learn) { base.learn(ec); }
  else { base.predict(ec); }
  ec.interactions = saved_interactions;
}

template <bool is_learn, VW::generate_func_t<VW::namespace_index> generate_func,
    VW::generate_func_t<VW::extent_term> generate_func_extents, bool leave_duplicate_interactions>
void transform_single_ex(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  data.update_extent_interactions_if_new_namespace_seen<generate_func_extents, leave_duplicate_interactions>(
      *ec.extent_interactions, ec.indices, ec.feature_space);

  auto* saved_extent_interactions = ec.extent_interactions;
  ec.extent_interactions = &data.generated_extent_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
    red_features.generated_extent_interactions = &data.generated_extent_interactions;
  }

  if (is_learn) { base.learn(ec); }
  else { base.predict(ec); }
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}

template <VW::generate_func_t<VW::namespace_index> generate_func,
    VW::generate_func_t<VW::extent_term> generate_func_extents, bool leave_duplicate_interactions>
void update(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  data.update_extent_interactions_if_new_namespace_seen<generate_func_extents, leave_duplicate_interactions>(
      *ec.extent_interactions, ec.indices, ec.feature_space);

  auto* saved_extent_interactions = ec.extent_interactions;
  ec.extent_interactions = &data.generated_extent_interactions;

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
    red_features.generated_extent_interactions = &data.generated_extent_interactions;
  }

  base.update(ec);
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}

template <VW::generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
void update(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
  }

  base.update(ec);
  ec.interactions = saved_interactions;
}

template <VW::generate_func_t<VW::namespace_index> generate_func, bool leave_duplicate_interactions>
inline void multipredict(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec, size_t count,
    size_t, VW::polyprediction* pred, bool finalize_predictions)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
  }

  base.multipredict(ec, 0, count, pred, finalize_predictions);
  ec.interactions = saved_interactions;
}

template <VW::generate_func_t<VW::namespace_index> generate_func,
    VW::generate_func_t<VW::extent_term> generate_func_extents, bool leave_duplicate_interactions>
inline void multipredict(VW::interactions_generator& data, VW::LEARNER::learner& base, VW::example& ec, size_t count,
    size_t, VW::polyprediction* pred, bool finalize_predictions)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  data.update_extent_interactions_if_new_namespace_seen<generate_func_extents, leave_duplicate_interactions>(
      *ec.extent_interactions, ec.indices, ec.feature_space);

  auto* saved_extent_interactions = ec.extent_interactions;
  ec.extent_interactions = &data.generated_extent_interactions;

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (data.store_in_reduction_features)
  {
    auto& red_features = ec.ex_reduction_features.template get<VW::large_action_space::las_reduction_features>();
    red_features.generated_interactions = &data.generated_interactions;
    red_features.generated_extent_interactions = &data.generated_extent_interactions;
  }

  base.multipredict(ec, 0, count, pred, finalize_predictions);
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::generate_interactions_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  bool leave_duplicate_interactions;
  bool store_in_reduction_features = false;
  option_group_definition new_options("[Reduction] Generate Interactions");
  new_options.add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
                      .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                            "duplicate: '-q ab -q ba' and a lot more in '-q ::'."));
  options.add_and_parse(new_options);

  auto interactions_spec_contains_wildcards = false;
  for (const auto& inter : all.interactions)
  {
    if (VW::contains_wildcard(inter))
    {
      interactions_spec_contains_wildcards = true;
      break;
    }
  }

  auto interactions_spec_contains_extent_wildcards = false;
  for (const auto& inter : all.extent_interactions)
  {
    if (VW::contains_wildcard(inter))
    {
      interactions_spec_contains_extent_wildcards = true;
      break;
    }
  }

  // If there are no wildcards, then no expansion is required.
  // ccb_explore_adf adds a wildcard post setup and so this reduction must be turned on.
  if (!(interactions_spec_contains_wildcards || interactions_spec_contains_extent_wildcards ||
          options.was_supplied("ccb_explore_adf")))
  {
    return nullptr;
  }

  if (options.was_supplied("large_action_space")) { store_in_reduction_features = true; }

  using learn_pred_func_t = void (*)(VW::interactions_generator&, VW::LEARNER::learner&, VW::example&);
  using multipredict_func_t = void (*)(
      VW::interactions_generator&, VW::LEARNER::learner&, VW::example&, size_t, size_t, VW::polyprediction*, bool);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  learn_pred_func_t update_func;
  multipredict_func_t multipredict_func;

  if (leave_duplicate_interactions)
  {
    if (interactions_spec_contains_extent_wildcards)
    {
      learn_func = transform_single_ex<true,  // is_learn
          VW::details::generate_namespace_permutations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_permutations_with_repetition<extent_term>,  // generate_fn<extent_term>
          true                                                                        // leave_duplicate_interactions
          >;
      pred_func = transform_single_ex<false,  // is_learn
          VW::details::generate_namespace_permutations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_permutations_with_repetition<extent_term>,  // generate_fn<extent_term>
          true                                                                        // leave_duplicate_interactions
          >;
      update_func = update<VW::details::generate_namespace_permutations_with_repetition<
                               VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_permutations_with_repetition<extent_term>,  // generate_fn<extent_term>
          true>;
      multipredict_func = multipredict<VW::details::generate_namespace_permutations_with_repetition<
                                           VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_permutations_with_repetition<extent_term>,  // generate_fn<extent_term>
          true>;
    }
    else
    {
      learn_func = transform_single_ex<true,  // is_learn
          VW::details::generate_namespace_permutations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          true                       // leave_duplicate_interactions
          >;
      pred_func = transform_single_ex<false,  // is_learn
          VW::details::generate_namespace_permutations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          true                       // leave_duplicate_interactions
          >;
      update_func = update<VW::details::generate_namespace_permutations_with_repetition<
                               VW::namespace_index>,  // generate_fn<VW::namespace_index>
          true                                        // leave_duplicate_interactions
          >;
      multipredict_func = multipredict<VW::details::generate_namespace_permutations_with_repetition<
                                           VW::namespace_index>,  // generate_fn<VW::namespace_index>
          true                                                    // leave_duplicate_interactions
          >;
    }
  }
  else
  {
    if (interactions_spec_contains_extent_wildcards)
    {
      learn_func = transform_single_ex<true,  // is_learn
          VW::details::generate_namespace_combinations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_combinations_with_repetition<extent_term>,  // generate_fn<extent_term>
          false                                                                       // leave_duplicate_interactions
          >;
      pred_func = transform_single_ex<false,  // is_learn
          VW::details::generate_namespace_combinations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_combinations_with_repetition<extent_term>,  // generate_fn<extent_term>
          false                                                                       // leave_duplicate_interactions
          >;
      update_func = update<VW::details::generate_namespace_combinations_with_repetition<
                               VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_combinations_with_repetition<extent_term>,  // generate_fn<extent_term>
          false                                                                       // leave_duplicate_interactions
          >;
      multipredict_func = multipredict<VW::details::generate_namespace_combinations_with_repetition<
                                           VW::namespace_index>,  // generate_fn<VW::namespace_index>
          VW::details::generate_namespace_combinations_with_repetition<extent_term>,  // generate_fn<extent_term>
          false                                                                       // leave_duplicate_interactions
          >;
    }
    else
    {
      learn_func = transform_single_ex<true,  // is_learn
          VW::details::generate_namespace_combinations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          false                      // leave_duplicate_interactions
          >;
      pred_func = transform_single_ex<false,  // is_learn
          VW::details::generate_namespace_combinations_with_repetition<
              VW::namespace_index>,  // generate_fn<VW::namespace_index>
          false                      // leave_duplicate_interactions
          >;
      update_func = update<VW::details::generate_namespace_combinations_with_repetition<
                               VW::namespace_index>,  // generate_fn<VW::namespace_index>
          false                                       // leave_duplicate_interactions
          >;
      multipredict_func = multipredict<VW::details::generate_namespace_combinations_with_repetition<
                                           VW::namespace_index>,  // generate_fn<VW::namespace_index>
          false                                                   // leave_duplicate_interactions
          >;
    }
  }

  auto data = VW::make_unique<VW::interactions_generator>();
  data->store_in_reduction_features = store_in_reduction_features;
  auto base = require_singleline(stack_builder.setup_base_learner());
  auto l = make_reduction_learner(
      std::move(data), base, learn_func, pred_func, stack_builder.get_setupfn_name(generate_interactions_setup))
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .set_update(update_func)
               .set_multipredict(multipredict_func)
               .build();
  return l;
}
