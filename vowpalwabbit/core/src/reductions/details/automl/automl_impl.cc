// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/automl_impl.h"

#include "vw/common/vw_exception.h"
#include "vw/core/estimators/confidence_sequence_robust.h"
#include "vw/core/multi_model_utils.h"

/*
This reduction implements the ChaCha algorithm from page 5 of the following paper:
https://arxiv.org/pdf/2106.04815.pdf

There are two key differences between this implementation and the algorithm described
in the paper. First, the paper assumes all knowledge (of examples and namespaces) is
known at the start, whereas that information is gathered as we process examples in reality.
Second, this algorithm follows the pattern Schedule -> Update Champ -> Learn, whereas
the paper follows the pattern Schedule -> Learn -> Update Champ. This should not make a
functional difference. Nearly all of the variables and functions described in the
algorithm live in the config_manager struct. The following provides a translation of
terms used in the paper to this implementation:

Variables from ChaCha:
c_init -> 0: The first champ is initialized to the 0th indexed set of weights
b(budget) -> max_live_configs: The weights/configs to learn on
C -> current_champ: The weight index of the current champ
S -> configs: The entire set of challengers/configs generated so far
B(+ C) -> estimators: The live challengers (including the champ) with statistics

Functions from ChaCha:
ConfigOracle -> config_oracle(): Generates new configs based on champ
Schedule -> schedule(): Swaps out / manages live configs on each step
Schedule.Choose -> calc_priority(): Determines priority when selecting next live config
Predict/Incur Loss -> offset_learn(): Updates weights and learns on live configs
Better -> better(): Changes champ if challenger is better
Worse -> worse(): Removes challenger is much worse than champ

New Concepts:
The 'eligible_to_inactivate' flag within socred_config is an implementation of the median
function in Algorithm 2 of Chacha. Basically we set half (or some number with the
--priority_challengers flag) of the live configs as regular, and the other half as priority.
The champ is a priority config, but is used for predictions while the other configs are not.
When a priority config runs out of lease, its lease it doubled and it's able to keep running.
When a regular config runs out of lease, we first compare it's lower_bound to each of the
priority config's ips. If the lower bound is higher, we'll swap the priorities (priority
becomes regular, regular becomes priority), then double its lease and keep running. If the
lower_bound of the regular config can't beat the ips of any priority configs, then it is
swapped out for some new config on the index_queue. The idea is that some live slots should
be used for priority challengers to have more time to beat the champ, while others are used
to swap out and try new configs more consistently.
*/

namespace VW
{
namespace reductions
{
namespace automl
{
// config_manager is a state machine (config_manager_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own learner.learn(). see learn_automl(...)
template <typename config_oracle_impl, typename estimator_impl>
interaction_config_manager<config_oracle_impl, estimator_impl>::interaction_config_manager(uint64_t default_lease,
    uint64_t max_live_configs, std::shared_ptr<VW::rand_state> rand_state, uint64_t priority_challengers,
    const std::string& interaction_type, const std::string& oracle_type, dense_parameters& weights,
    priority_func calc_priority, double automl_significance_level, VW::io::logger* logger, uint32_t& wpp, bool ccb_on,
    config_type conf_type, std::string trace_prefix, bool reward_as_cost, double tol_x, bool is_brentq)
    : default_lease(default_lease)
    , max_live_configs(max_live_configs)
    , priority_challengers(priority_challengers)
    , weights(weights)
    , automl_significance_level(automl_significance_level)
    , logger(logger)
    , wpp(wpp)
    , _ccb_on(ccb_on)
    , _config_oracle(config_oracle_impl(
          default_lease, std::move(calc_priority), interaction_type, oracle_type, rand_state, conf_type))
    , reward_as_cost(reward_as_cost)
    , tol_x(tol_x)
    , is_brentq(is_brentq)
{
  if (trace_prefix != "")
  {
    champ_log_file = VW::make_unique<std::ofstream>(trace_prefix + ".champ_change.csv");
    *champ_log_file << "state, example_count, slot_id, config_type, ns_elements" << std::endl;
    inputlabel_log_file = VW::make_unique<std::ofstream>(trace_prefix + "input_examples.csv");
    *inputlabel_log_file << "example_count, logged_action, logged_probability, weight, reward" << std::endl;
  }
  insert_starting_configuration(estimators, _config_oracle, automl_significance_level, tol_x, is_brentq);
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::insert_starting_configuration(
    estimator_vec_t<estimator_impl>& estimators, config_oracle_impl& config_oracle, const double sig_level,
    const double tol_x, bool is_brentq)
{
  assert(config_oracle.index_queue.size() == 0);
  assert(config_oracle.configs.size() == 0);

  config_oracle.insert_starting_configuration();

  assert(config_oracle.index_queue.size() == 0);
  assert(config_oracle.configs.size() >= 1);

  config_oracle.configs[0].state = VW::reductions::automl::config_state::New;
  estimators.emplace_back(std::make_pair(
      aml_estimator<estimator_impl>(tol_x, is_brentq, sig_level), estimator_impl(tol_x, is_brentq, sig_level)));
}

template <typename config_oracle_impl, typename estimator_impl>
bool interaction_config_manager<config_oracle_impl, estimator_impl>::swap_eligible_to_inactivate(
    estimator_vec_t<estimator_impl>& estimators, uint64_t live_slot)
{
  const uint64_t current_champ = 0;
  for (uint64_t other_live_slot = 0; other_live_slot < estimators.size(); ++other_live_slot)
  {
    if (!estimators[other_live_slot].first.eligible_to_inactivate && other_live_slot != current_champ)
    {
      if (aml_estimator<estimator_impl>::better(
              estimators[live_slot].first._estimator, estimators[other_live_slot].first._estimator))
      {
        estimators[live_slot].first.eligible_to_inactivate = false;
        estimators[other_live_slot].first.eligible_to_inactivate = true;
        return true;
      }
    }
  }
  return false;
}

// This function defines the logic update the live configs' leases, and to swap out
// new configs when the lease runs out.
template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::schedule()
{
  for (uint64_t live_slot = 0; live_slot < max_live_configs; ++live_slot)
  {
    bool need_new_estimator = estimators.size() <= live_slot;
    /*
    Scheduling a new live config is necessary in 3 cases:
    1. We have not reached the maximum number of live configs yet
    2. The current live config has been removed due to Chacha's worse function
    3. A config has reached its lease
    */
    if (need_new_estimator ||
        _config_oracle.configs[estimators[live_slot].first.config_index].state ==
            VW::reductions::automl::config_state::Removed ||
        estimators[live_slot].first._estimator.update_count >=
            _config_oracle.configs[estimators[live_slot].first.config_index].lease)
    {
      // Double the lease check swap for eligible_to_inactivate configs
      if (!need_new_estimator &&
          _config_oracle.configs[estimators[live_slot].first.config_index].state ==
              VW::reductions::automl::config_state::Live)
      {
        _config_oracle.configs[estimators[live_slot].first.config_index].lease *= 2;
        if (!estimators[live_slot].first.eligible_to_inactivate || swap_eligible_to_inactivate(estimators, live_slot))
        {
          continue;
        }
      }
      // Skip over removed configs in index queue, and do nothing we we run out of eligible configs
      while (!_config_oracle.index_queue.empty() &&
          _config_oracle.configs[_config_oracle.index_queue.top().second].state ==
              VW::reductions::automl::config_state::Removed)
      {
        _config_oracle.index_queue.pop();
      }
      if (_config_oracle.index_queue.empty() && !_config_oracle.repopulate_index_queue(ns_counter)) { continue; }

      // Only inactivate current config if lease is reached
      if (!need_new_estimator &&
          _config_oracle.configs[estimators[live_slot].first.config_index].state ==
              VW::reductions::automl::config_state::Live)
      {
        _config_oracle.configs[estimators[live_slot].first.config_index].state =
            VW::reductions::automl::config_state::Inactive;
      }

      assert(live_slot < max_live_configs);
      // fetch config from the queue, and apply it current live slot
      apply_config_at_slot(estimators, _config_oracle.configs, live_slot,
          config_oracle_impl::choose(_config_oracle.index_queue), automl_significance_level, tol_x, is_brentq,
          priority_challengers);
      if (champ_log_file)
      {
        *champ_log_file << "APPLY_CONFIG," << total_learn_count << "," << live_slot << ","
                        << to_string(_config_oracle._conf_type) << ","
                        << util::elements_to_string(
                               _config_oracle.configs[estimators[live_slot].first.config_index].elements, " ")
                        << std::endl;
      }

      // copy the weights of the champ to the new slot
      VW::reductions::multi_model::move_innermost_offsets(weights, current_champ, live_slot, wpp, max_live_configs);
      // Regenerate interactions each time an exclusion is swapped in
      ns_based_config::apply_config_to_interactions(_ccb_on, ns_counter, _config_oracle._interaction_type,
          _config_oracle.configs[estimators[live_slot].first.config_index],
          estimators[live_slot].first.live_interactions);
    }
  }
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::apply_config_at_slot(
    estimator_vec_t<estimator_impl>& estimators, std::vector<ns_based_config>& configs, const uint64_t live_slot,
    const uint64_t config_index, const double sig_level, const double tol_x, bool is_brentq,
    const uint64_t priority_challengers)
{
  // Allocate new estimator if we haven't reached maximum yet
  if (estimators.size() <= live_slot)
  {
    estimators.emplace_back(std::make_pair(
        aml_estimator<estimator_impl>(tol_x, is_brentq, sig_level), estimator_impl(tol_x, is_brentq, sig_level)));
    if (live_slot > priority_challengers) { estimators.back().first.eligible_to_inactivate = true; }
  }
  assert(estimators.size() > live_slot);

  // Set all features of new live config
  estimators[live_slot].first._estimator.reset_stats();
  estimators[live_slot].second.reset_stats();

  estimators[live_slot].first.config_index = config_index;
  configs[config_index].state = VW::reductions::automl::config_state::Live;

  // We may also want to 0 out weights here? Currently keep all same in live_slot position
  // TODO: reset stats of gd, cb_adf, sd patch , to default.. what is default?
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::check_for_new_champ()
{
  bool champ_change = false;
  uint64_t old_champ_slot = current_champ;
  assert(old_champ_slot == 0);
  uint64_t winning_challenger_slot = 0;

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
  {
    if (live_slot == current_champ) { continue; }
    // If challenger is better ('better function from Chacha')
    if (aml_estimator<estimator_impl>::better(estimators[live_slot].first._estimator, estimators[live_slot].second))
    {
      champ_change = true;
      winning_challenger_slot = live_slot;
    }
    else if (worse())  // If challenger is worse ('worse function from Chacha')
    {
      _config_oracle.configs[estimators[live_slot].first.config_index].state =
          VW::reductions::automl::config_state::Removed;
    }
  }
  if (champ_change)
  {
    this->total_champ_switches++;

    /*
     * Note here that the wining challenger (and its weights) will be moved into slot 0, and the old
     * champion will move into slot 1. All other weights are no longer relevant and will later take on the
     * champ's weights. The current champion will always be in slot 0, so if the winning challenger is in
     * slot 3 with 5 live models, the following operations will occur:
     * w0 w1 w2 w3 w4
     * w3 w1 w2 w0 w4 // w3 are the weights for the winning challenger (new champion) and placed in slot 0
     * w3 w0 w2 w0 w4 // w0 are the old champ's weights and place in slot 1, other weights are irrelevant
     */

    // this is a swap, see last bool argument in move_innermost_offsets
    VW::reductions::multi_model::move_innermost_offsets(
        weights, winning_challenger_slot, old_champ_slot, wpp, max_live_configs, true);
    if (winning_challenger_slot != 1)
    {
      VW::reductions::multi_model::move_innermost_offsets(
          weights, winning_challenger_slot, 1, wpp, max_live_configs, false);
    }

    apply_new_champ(_config_oracle, winning_challenger_slot, estimators, priority_challengers, ns_counter);
    if (champ_log_file)
    {
      *champ_log_file << "CHAMP_SWITCH," << total_learn_count << "," << total_champ_switches << ","
                      << to_string(_config_oracle._conf_type) << ","
                      << util::elements_to_string(_config_oracle.configs[0].elements, " ") << std::endl;
    }
  }
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::apply_new_champ(config_oracle_impl& config_oracle,
    const uint64_t winning_challenger_slot, estimator_vec_t<estimator_impl>& estimators,
    const uint64_t priority_challengers, const std::map<namespace_index, uint64_t>& ns_counter)
{
  const uint64_t champ_slot = 0;

  while (!config_oracle.index_queue.empty()) { config_oracle.index_queue.pop(); };

  estimators[winning_challenger_slot].first.eligible_to_inactivate = false;
  if (priority_challengers > 1) { estimators[champ_slot].first.eligible_to_inactivate = false; }

  config_oracle.keep_best_two(estimators[winning_challenger_slot].first.config_index);

  estimators[winning_challenger_slot].first.config_index = 0;
  estimators[champ_slot].first.config_index = 1;

  auto champ_estimator = std::move(estimators[winning_challenger_slot]);
  auto old_champ_estimator = std::move(estimators[champ_slot]);

  estimators.clear();

  estimators.push_back(std::move(champ_estimator));
  estimators.push_back(std::move(old_champ_estimator));

  /*
   * These operations rearrange the scoring data to sync up the new champion and old champion. Assume the first
   * challenger (chal_1) is taking over the champ (old_champ). The estimators would have this configuration before a
   * champ change: slot_0: <old_champ_dummy, old_champ_dummy_track_champ> slot_1: <chal_1, chal_1_track_champ> Note
   * that the estimators <old_champ_dummy, old_champ_dummy_track_champ> are not updated since the champion does not
   * need to track itself. After the champ change, the same estimators will look like this: slot_0: <chal_1,
   * chal_1_track_champ> slot_1: <old_champ_dummy, old_champ_dummy_track_champ> We then want to move the statistics
   * from chal_1_track_champ to old_champ_dummy and chal_1 to old_champ_dummy_track_champ since they both share the
   * same horizons, but have swapped which is champion and which is challenger. While we want to swap these
   * statistics, we don't want to alter the other state such as interactions and config_index.
   */
  estimators[1].first = aml_estimator<estimator_impl>(std::move(estimators[0].second), estimators[1].first.config_index,
      estimators[1].first.eligible_to_inactivate, estimators[1].first.live_interactions);
  estimators[1].second = estimators[0].first._estimator;

  config_oracle.gen_configs(estimators[champ_slot].first.live_interactions, ns_counter);
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::do_learning(
    LEARNER::learner& base, multi_ex& ec, uint64_t live_slot)
{
  assert(live_slot < max_live_configs);
  // TODO: what to do if that slot is switched with a new config?
  std::swap(*_gd_normalized, per_live_model_state_double[live_slot * 3]);
  std::swap(*_gd_total_weight, per_live_model_state_double[live_slot * 3 + 1]);
  std::swap(*_sd_gravity, per_live_model_state_double[live_slot * 3 + 2]);
  std::swap(*_cb_adf_event_sum, per_live_model_state_uint64[live_slot * 2]);
  std::swap(*_cb_adf_action_sum, per_live_model_state_uint64[live_slot * 2 + 1]);
  for (example* ex : ec) { apply_config(ex, &estimators[live_slot].first.live_interactions); }
  if (!base.learn_returns_prediction) { base.predict(ec, live_slot); }
  base.learn(ec, live_slot);
  std::swap(*_gd_normalized, per_live_model_state_double[live_slot * 3]);
  std::swap(*_gd_total_weight, per_live_model_state_double[live_slot * 3 + 1]);
  std::swap(*_sd_gravity, per_live_model_state_double[live_slot * 3 + 2]);
  std::swap(*_cb_adf_event_sum, per_live_model_state_uint64[live_slot * 2]);
  std::swap(*_cb_adf_action_sum, per_live_model_state_uint64[live_slot * 2 + 1]);
}

template <typename config_oracle_impl, typename estimator_impl>
void interaction_config_manager<config_oracle_impl, estimator_impl>::process_example(const multi_ex& ec)
{
  bool new_ns_seen = count_namespaces(ec, ns_counter);
  // Regenerate interactions if new namespaces are seen
  if (new_ns_seen)
  {
    for (uint64_t live_slot = 0; live_slot < estimators.size(); ++live_slot)
    {
      auto& curr_config = _config_oracle.configs[estimators[live_slot].first.config_index];
      auto& interactions = estimators[live_slot].first.live_interactions;
      ns_based_config::apply_config_to_interactions(
          _ccb_on, ns_counter, _config_oracle._interaction_type, curr_config, interactions);
    }

    if (_config_oracle.configs[current_champ].state == VW::reductions::automl::config_state::New)
    {
      _config_oracle.configs[current_champ].state = VW::reductions::automl::config_state::Live;
      _config_oracle.gen_configs(estimators[current_champ].first.live_interactions, ns_counter);
    }
  }
}

template class interaction_config_manager<config_oracle<oracle_rand_impl>, VW::estimators::confidence_sequence_robust>;
template class interaction_config_manager<config_oracle<one_diff_impl>, VW::estimators::confidence_sequence_robust>;
template class interaction_config_manager<config_oracle<champdupe_impl>, VW::estimators::confidence_sequence_robust>;
template class interaction_config_manager<config_oracle<one_diff_inclusion_impl>,
    VW::estimators::confidence_sequence_robust>;
template class interaction_config_manager<config_oracle<qbase_cubic>, VW::estimators::confidence_sequence_robust>;

template <typename CMType>
void automl<CMType>::one_step(LEARNER::learner& base, multi_ex& ec, VW::cb_class& logged, uint64_t labelled_action)
{
  cm->total_learn_count++;
  cm->process_example(ec);
  cm->schedule();
  offset_learn(base, ec, logged, labelled_action);
  cm->check_for_new_champ();
}

template <typename CMType>
void automl<CMType>::offset_learn(LEARNER::learner& base, multi_ex& ec, VW::cb_class& logged, uint64_t labelled_action)
{
  interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  const float w = logged.probability > 0 ? 1 / logged.probability : 0;
  const float r = cm->reward_as_cost ? logged.cost : -logged.cost;

  if (cm->inputlabel_log_file)
  {
    *cm->inputlabel_log_file << cm->total_learn_count << "," << logged.action << "," << logged.probability << "," << w
                             << "," << r << std::endl;
  }

  int64_t live_slot = 0;
  int64_t current_champ = static_cast<int64_t>(cm->current_champ);
  assert(current_champ == 0);

  auto restore_guard = VW::scope_exit(
      [&ec, &incoming_interactions]()
      {
        for (example* ex : ec) { ex->interactions = incoming_interactions; }
      });

  // Learn and update estimators of challengers
  for (int64_t current_slot_index = 1; static_cast<size_t>(current_slot_index) < cm->estimators.size();
       ++current_slot_index)
  {
    if (!debug_reverse_learning_order) { live_slot = current_slot_index; }
    else { live_slot = cm->estimators.size() - current_slot_index; }
    cm->do_learning(base, ec, live_slot);
    cm->estimators[live_slot].first._estimator.update(ec[0]->pred.a_s[0].action == labelled_action ? w : 0, r);
  }

  // ** Note: champ learning is done after to ensure correct feature count in gd **
  // Learn and get action of champ
  cm->do_learning(base, ec, current_champ);

  if (ec.size() < 1) { return; }

  for (live_slot = 1; static_cast<size_t>(live_slot) < cm->estimators.size(); ++live_slot)
  {
    cm->estimators[live_slot].second.update(ec[0]->pred.a_s[0].action == labelled_action ? w : 0, r);
    auto& curr = cm->estimators[live_slot];
    if (log_file)
    {
      *log_file << cm->total_learn_count << "," << live_slot << "," << cm->total_champ_switches << ","
                << curr.first._estimator.lower_bound() << "," << curr.first._estimator.upper_bound() << ","
                << curr.second.lower_bound() << "," << curr.second.upper_bound() << std::endl;
    }
  }
}

template class automl<
    interaction_config_manager<config_oracle<oracle_rand_impl>, VW::estimators::confidence_sequence_robust>>;
template class automl<
    interaction_config_manager<config_oracle<one_diff_impl>, VW::estimators::confidence_sequence_robust>>;
template class automl<
    interaction_config_manager<config_oracle<champdupe_impl>, VW::estimators::confidence_sequence_robust>>;
template class automl<
    interaction_config_manager<config_oracle<one_diff_inclusion_impl>, VW::estimators::confidence_sequence_robust>>;
template class automl<
    interaction_config_manager<config_oracle<qbase_cubic>, VW::estimators::confidence_sequence_robust>>;

}  // namespace automl
}  // namespace reductions
}  // namespace VW
