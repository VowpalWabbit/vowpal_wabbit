// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <sstream>
#include <cfloat>
#include "generate_interactions.h"
#include "interactions.h"
#include "reductions.h"
#include "v_array.h"

#include "io/logger.h"
#include "interactions.h"
#include "vw_math.h"
#include <algorithm>
#include <iterator>

using namespace VW::config;

template <bool is_learn, INTERACTIONS::generate_func_t<namespace_index> generate_func,
    bool leave_duplicate_interactions>
void transform_single_ex(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
  ec.interactions = saved_interactions;
}

template <bool is_learn, INTERACTIONS::generate_func_t<namespace_index> generate_func,
    INTERACTIONS::generate_func_t<extent_term> generate_func_extents, bool leave_duplicate_interactions>
void transform_single_ex(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec)
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

  if (is_learn) { base.learn(ec); }
  else
  {
    base.predict(ec);
  }
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}

template <INTERACTIONS::generate_func_t<namespace_index> generate_func,
    INTERACTIONS::generate_func_t<extent_term> generate_func_extents, bool leave_duplicate_interactions>
void update(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec)
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
  base.update(ec);
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}

template <INTERACTIONS::generate_func_t<namespace_index> generate_func, bool leave_duplicate_interactions>
void update(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;
  base.update(ec);
  ec.interactions = saved_interactions;
}

template <INTERACTIONS::generate_func_t<namespace_index> generate_func, bool leave_duplicate_interactions>
inline void multipredict(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec,
    size_t count, size_t, polyprediction* pred, bool finalize_predictions)
{
  // We pass *ec.interactions here BUT the contract is that this does not change...
  data.update_interactions_if_new_namespace_seen<generate_func, leave_duplicate_interactions>(
      *ec.interactions, ec.indices);

  auto* saved_interactions = ec.interactions;
  ec.interactions = &data.generated_interactions;
  base.multipredict(ec, 0, count, pred, finalize_predictions);
  ec.interactions = saved_interactions;
}

template <INTERACTIONS::generate_func_t<namespace_index> generate_func,
    INTERACTIONS::generate_func_t<extent_term> generate_func_extents, bool leave_duplicate_interactions>
inline void multipredict(INTERACTIONS::interactions_generator& data, VW::LEARNER::single_learner& base, example& ec,
    size_t count, size_t, polyprediction* pred, bool finalize_predictions)
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
  base.multipredict(ec, 0, count, pred, finalize_predictions);
  ec.interactions = saved_interactions;
  ec.extent_interactions = saved_extent_interactions;
}

VW::LEARNER::base_learner* generate_interactions_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();
  bool leave_duplicate_interactions;
  option_group_definition new_options("Generate interactions");
  new_options.add(make_option("leave_duplicate_interactions", leave_duplicate_interactions)
                      .help("Don't remove interactions with duplicate combinations of namespaces. For ex. this is a "
                            "duplicate: '-q ab -q ba' and a lot more in '-q ::'."));
  options.add_and_parse(new_options);

  auto interactions_spec_contains_wildcards = false;
  for (const auto& inter : all.interactions)
  {
    if (INTERACTIONS::contains_wildcard(inter))
    {
      interactions_spec_contains_wildcards = true;
      break;
    }
  }

  auto interactions_spec_contains_extent_wildcards = false;
  for (const auto& inter : all.extent_interactions)
  {
    if (INTERACTIONS::contains_wildcard(inter))
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

  using learn_pred_func_t = void (*)(INTERACTIONS::interactions_generator&, VW::LEARNER::single_learner&, example&);
  using multipredict_func_t = void (*)(INTERACTIONS::interactions_generator&, VW::LEARNER::single_learner&, example&,
      size_t, size_t, polyprediction*, bool);
  learn_pred_func_t learn_func;
  learn_pred_func_t pred_func;
  learn_pred_func_t update_func;
  multipredict_func_t multipredict_func;

  if (leave_duplicate_interactions)
  {
    if (interactions_spec_contains_extent_wildcards)
    {
      learn_func =
          transform_single_ex<true, INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>,
              INTERACTIONS::generate_namespace_permutations_with_repetition<extent_term>, true>;
      pred_func =
          transform_single_ex<false, INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>,
              INTERACTIONS::generate_namespace_permutations_with_repetition<extent_term>, true>;
      update_func = update<INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>,
          INTERACTIONS::generate_namespace_permutations_with_repetition<extent_term>, true>;
      multipredict_func = multipredict<INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>,
          INTERACTIONS::generate_namespace_permutations_with_repetition<extent_term>, true>;
    }
    else
    {
      learn_func = transform_single_ex<true,
          INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>, true>;
      pred_func = transform_single_ex<false,
          INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>, true>;
      update_func = update<INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>, true>;
      multipredict_func =
          multipredict<INTERACTIONS::generate_namespace_permutations_with_repetition<namespace_index>, true>;
    }
  }
  else
  {
    if (interactions_spec_contains_extent_wildcards)
    {
      learn_func =
          transform_single_ex<true, INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>,
              INTERACTIONS::generate_namespace_combinations_with_repetition<extent_term>, false>;
      pred_func =
          transform_single_ex<false, INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>,
              INTERACTIONS::generate_namespace_combinations_with_repetition<extent_term>, false>;
      update_func = update<INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>,
          INTERACTIONS::generate_namespace_combinations_with_repetition<extent_term>, false>;
      multipredict_func = multipredict<INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>,
          INTERACTIONS::generate_namespace_combinations_with_repetition<extent_term>, false>;
    }
    else
    {
      learn_func = transform_single_ex<true,
          INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>, false>;
      pred_func = transform_single_ex<false,
          INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>, false>;
      update_func = update<INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>, false>;
      multipredict_func =
          multipredict<INTERACTIONS::generate_namespace_combinations_with_repetition<namespace_index>, false>;
    }
  }

  auto data = VW::make_unique<INTERACTIONS::interactions_generator>();
  auto* base = as_singleline(stack_builder.setup_base_learner());
  auto* l = VW::LEARNER::make_reduction_learner(
      std::move(data), base, learn_func, pred_func, stack_builder.get_setupfn_name(generate_interactions_setup))
                .set_learn_returns_prediction(base->learn_returns_prediction)
                .set_update(update_func)
                .set_multipredict(multipredict_func)
                .build();
  return VW::LEARNER::make_base(*l);
}
