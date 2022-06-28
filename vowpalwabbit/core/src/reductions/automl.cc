// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl.h"

#include "vw/config/options.h"
#include "vw/core/constant.h"  // NUM_NAMESPACES
#include "vw/core/debug_log.h"
#include "vw/core/interactions.h"
#include "vw/core/model_utils.h"
#include "vw/core/rand_state.h"
#include "vw/core/setup_base.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;

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
B(+ C) -> scores: The live challengers (including the champ) with statistics

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

namespace
{
constexpr uint64_t MAX_CONFIGS = 10;
constexpr uint64_t CONFIGS_PER_CHAMP_CHANGE = 5;

// fail if incompatible reductions got setup
// todo: audit if they reference global all interactions
void fail_if_enabled(VW::workspace& all, const std::set<std::string>& not_compat)
{
  std::vector<std::string> enabled_reductions;
  if (all.l != nullptr) { all.l->get_enabled_reductions(enabled_reductions); }

  for (const auto& reduction : enabled_reductions)
  {
    if (not_compat.count(reduction) > 0) THROW("automl does not yet support this reduction: " + reduction);
  }
}

std::string ns_to_str(unsigned char ns)
{
  if (ns == constant_namespace) { return "[constant]"; }
  else if (ns == ccb_slot_namespace)
  {
    return "[ccbslot]";
  }
  else if (ns == ccb_id_namespace)
  {
    return "[ccbid]";
  }
  else if (ns == wildcard_namespace)
  {
    return "[wild]";
  }
  else if (ns == default_namespace)
  {
    return "[default]";
  }
  else
  {
    return std::string(1, ns);
  }
}

std::string interaction_vec_t_to_string(
    const VW::reductions::automl::interaction_vec_t& interactions, const std::string& interaction_type)
{
  std::stringstream ss;
  for (const std::vector<VW::namespace_index>& v : interactions)
  {
    interaction_type == "quadratic" ? ss << "-q " : ss << "-cubic ";
    for (VW::namespace_index c : v) { ss << ns_to_str(c); }
    ss << " ";
  }
  return ss.str();
}

std::string exclusions_to_string(const std::set<std::vector<VW::namespace_index>>& exclusions)
{
  const char* const delim = ", ";
  std::stringstream ss;
  size_t total = exclusions.size();
  size_t count = 0;
  ss << "{";
  for (auto const& x : exclusions)
  {
    ss << "[";
    if (!x.empty())
    {
      auto i = x.begin(), x_last = std::prev(x.end());
      for (; i != x_last; ++i) { ss << "\"" << ns_to_str(*i) << "\"" << delim; }
      ss << "\"" << ns_to_str(*x_last) << "\"";
    }
    count += 1;
    ss << "]";
    if (count < total) { ss << delim; }
  }
  ss << "}";
  return ss.str();
}
}  // namespace

namespace VW
{
namespace reductions
{
namespace automl
{
void aml_score::persist(
    metric_sink& metrics, const std::string& suffix, bool verbose, const std::string& interaction_type)
{
  VW::scored_config::persist(metrics, suffix);
  metrics.set_uint("conf_idx" + suffix, config_index);
  if (verbose)
  { metrics.set_string("interactions" + suffix, ::interaction_vec_t_to_string(live_interactions, interaction_type)); }
}

template <typename CMType>
void automl<CMType>::one_step(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
{
  cm->total_learn_count++;
  switch (current_state)
  {
    case VW::reductions::automl::automl_state::Collecting:
      cm->pre_process(ec);
      cm->config_oracle();
      offset_learn(base, ec, logged, labelled_action);
      current_state = VW::reductions::automl::automl_state::Experimenting;
      break;

    case VW::reductions::automl::automl_state::Experimenting:
      cm->pre_process(ec);
      cm->schedule();
      offset_learn(base, ec, logged, labelled_action);
      cm->update_champ();
      break;

    default:
      break;
  }
}

// config_manager is a state machine (config_manager_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own base_learner.learn(). see learn_automl(...)
interaction_config_manager::interaction_config_manager(uint64_t global_lease, uint64_t max_live_configs,
    std::shared_ptr<VW::rand_state> rand_state, uint64_t priority_challengers, bool keep_configs,
    std::string interaction_type, std::string oracle_type, dense_parameters& weights, priority_func* calc_priority,
    double automl_significance_level, double automl_estimator_decay, VW::io::logger* logger, uint32_t& wpp)
    : global_lease(global_lease)
    , max_live_configs(max_live_configs)
    , random_state(std::move(rand_state))
    , priority_challengers(priority_challengers)
    , keep_configs(keep_configs)
    , interaction_type(std::move(interaction_type))
    , oracle_type(std::move(oracle_type))
    , weights(weights)
    , calc_priority(calc_priority)
    , automl_significance_level(automl_significance_level)
    , automl_estimator_decay(automl_estimator_decay)
    , logger(logger)
    , wpp(wpp)
{
  configs[0] = exclusion_config(global_lease);
  configs[0].state = VW::reductions::automl::config_state::Live;
  scores.emplace_back(automl_significance_level, automl_estimator_decay);
  champ_scores.emplace_back(automl_significance_level, automl_estimator_decay);
  ++valid_config_size;
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.

void interaction_config_manager::gen_interactions(uint64_t live_slot)
{
  if (interaction_type == "quadratic")
  {
    auto& exclusions = configs[scores[live_slot].config_index].exclusions;
    auto& interactions = scores[live_slot].live_interactions;
    if (!interactions.empty()) { interactions.clear(); }
    for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
    {
      auto idx1 = (*it).first;
      for (auto jt = it; jt != ns_counter.end(); ++jt)
      {
        auto idx2 = (*jt).first;
        std::vector<namespace_index> idx{idx1, idx2};
        if (exclusions.find(idx) == exclusions.end()) { interactions.push_back({idx1, idx2}); }
      }
    }
  }
  else if (interaction_type == "cubic")
  {
    auto& exclusions = configs[scores[live_slot].config_index].exclusions;
    auto& interactions = scores[live_slot].live_interactions;
    if (!interactions.empty()) { interactions.clear(); }
    for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
    {
      auto idx1 = (*it).first;
      for (auto jt = it; jt != ns_counter.end(); ++jt)
      {
        auto idx2 = (*jt).first;
        for (auto kt = jt; kt != ns_counter.end(); ++kt)
        {
          auto idx3 = (*kt).first;
          std::vector<namespace_index> idx{idx1, idx2, idx3};
          if (exclusions.find(idx) == exclusions.end()) { interactions.push_back({idx1, idx2, idx3}); }
        }
      }
    }
  }
  else
  {
    THROW("Unknown interaction type.");
  }
}

// This function will process an incoming multi_ex, update the namespace_counter,
// log if new namespaces are encountered, and regenerate interactions based on
// newly seen namespaces.
void interaction_config_manager::pre_process(const multi_ex& ecs)
{
  // Count all namepsace seen in current example
  bool new_ns_seen = false;
  for (const example* ex : ecs)
  {
    for (const auto& ns : ex->indices)
    {
      if (!INTERACTIONS::is_interaction_ns(ns)) { continue; }
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_ns_seen = true; }
    }
  }

  // Regenerate interactions if new namespaces are seen
  if (new_ns_seen)
  {
    for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot) { gen_interactions(live_slot); }
  }
}
// Helper function to insert new configs from oracle into map of configs as well as index_queue.
// Handles creating new config with exclusions or overwriting stale configs to avoid reallocation.
void interaction_config_manager::insert_config(std::set<std::vector<namespace_index>>&& new_exclusions)
{
  // Note that configs are never actually cleared, but valid_config_size is set to 0 instead to denote that
  // configs have become stale. Here we try to write over stale configs with new configs, and if no stale
  // configs exist we'll generate a new one.
  if (valid_config_size < configs.size())
  {
    configs[valid_config_size].exclusions = std::move(new_exclusions);
    configs[valid_config_size].lease = global_lease;
    configs[valid_config_size].state = VW::reductions::automl::config_state::New;
  }
  else
  {
    configs[valid_config_size] = exclusion_config(global_lease);
    configs[valid_config_size].exclusions = std::move(new_exclusions);
  }
  float priority = (*calc_priority)(configs[valid_config_size], ns_counter);
  index_queue.push(std::make_pair(priority, valid_config_size));
  ++valid_config_size;
}

// This will generate configs based on the current champ. These configs will be
// stored as 'exclusions.' The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
// TODO: Add logic to avoid duplicate configs (could be very costly)
void interaction_config_manager::config_oracle()
{
  auto& champ_interactions = scores[current_champ].live_interactions;
  if (oracle_type == "rand")
  {
    for (uint64_t i = 0; i < CONFIGS_PER_CHAMP_CHANGE; ++i)
    {
      uint64_t rand_ind = static_cast<uint64_t>(random_state->get_and_update_random() * champ_interactions.size());
      std::set<std::vector<namespace_index>> new_exclusions(configs[scores[current_champ].config_index].exclusions);
      if (interaction_type == "quadratic")
      {
        namespace_index ns1 = champ_interactions[rand_ind][0];
        namespace_index ns2 = champ_interactions[rand_ind][1];
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
      else if (interaction_type == "cubic")
      {
        namespace_index ns1 = champ_interactions[rand_ind][0];
        namespace_index ns2 = champ_interactions[rand_ind][1];
        namespace_index ns3 = champ_interactions[rand_ind][2];
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        new_exclusions.insert(idx);
      }
      else
      {
        THROW("Unknown interaction type.");
      }
      insert_config(std::move(new_exclusions));
    }
  }
  /*
   * Example of one_diff oracle:
   * Say we have namespaces {a,b,c}, so all quadratic interactions are {aa, ab, ac, bb, bc, cc}. If the champ has
   * an exclusion set {aa, ab}, then the champs interactions would be {ac, bb, bc, cc}. We want to generate all
   * configs with a distance of 1 from the champ, meaning all sets with one less exclusion, and all sets with one
   * more exclusion. So the first part looks through the champs interaction set, and creates new configs which add
   * one of those to the current exclusion set (eg it will generate {aa, ab, ac}, {aa, ab, bb}, {aa, ab, bc},
   * {aa, ab, cc}). Then the second part will create all exclusion sets which remove one (eg {aa} and {ab}).
   */
  else if (oracle_type == "one_diff")
  {
    // Add one exclusion (for each interaction)
    for (auto& interaction : champ_interactions)
    {
      std::set<std::vector<namespace_index>> new_exclusions(configs[scores[current_champ].config_index].exclusions);
      if (interaction_type == "quadratic")
      {
        namespace_index ns1 = interaction[0];
        namespace_index ns2 = interaction[1];
        std::vector<namespace_index> idx{ns1, ns2};
        new_exclusions.insert(idx);
      }
      else if (interaction_type == "cubic")
      {
        namespace_index ns1 = interaction[0];
        namespace_index ns2 = interaction[1];
        namespace_index ns3 = interaction[2];
        std::vector<namespace_index> idx{ns1, ns2, ns3};
        new_exclusions.insert(idx);
      }
      else
      {
        THROW("Unknown interaction type.");
      }
      insert_config(std::move(new_exclusions));
    }
    // Remove one exclusion (for each exclusion)
    for (auto& ns_pair : configs[scores[current_champ].config_index].exclusions)
    {
      std::set<std::vector<namespace_index>> new_exclusions(configs[scores[current_champ].config_index].exclusions);
      new_exclusions.erase(ns_pair);
      insert_config(std::move(new_exclusions));
    }
  }
  else if (oracle_type == "champdupe")
  {
    for (uint64_t i = 0; i < max_live_configs; ++i)
    { insert_config(std::set<std::vector<namespace_index>>(configs[scores[current_champ].config_index].exclusions)); }
  }
  else
  {
    THROW("Unknown oracle type.");
  }
}
// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
bool interaction_config_manager::repopulate_index_queue()
{
  for (size_t i = 0; i < valid_config_size; ++i)
  {
    // Only re-add if not removed and not live
    if (configs[i].state == VW::reductions::automl::config_state::New ||
        configs[i].state == VW::reductions::automl::config_state::Inactive)
    {
      float priority = (*calc_priority)(configs[i], ns_counter);
      index_queue.push(std::make_pair(priority, i));
    }
  }
  return !index_queue.empty();
}

bool interaction_config_manager::swap_eligible_to_inactivate(uint64_t live_slot)
{
  for (uint64_t other_live_slot = 0; other_live_slot < scores.size(); ++other_live_slot)
  {
    bool better = lb_trick ? scores[live_slot].lower_bound() > (1.f - scores[other_live_slot].lower_bound()) : scores[live_slot].lower_bound() > scores[other_live_slot].upper_bound();
    if (!scores[other_live_slot].eligible_to_inactivate && other_live_slot != current_champ &&
        better)
    {
      scores[live_slot].eligible_to_inactivate = false;
      scores[other_live_slot].eligible_to_inactivate = true;
      return true;
    }
  }
  return false;
}

// This function defines the logic update the live configs' leases, and to swap out
// new configs when the lease runs out.
void interaction_config_manager::schedule()
{
  for (uint64_t live_slot = 0; live_slot < max_live_configs; ++live_slot)
  {
    bool need_new_score = scores.size() <= live_slot;
    /*
    Scheduling a new live config is necessary in 3 cases:
    1. We have not reached the maximum number of live configs yet
    2. The current live config has been removed due to Chacha's worse function
    3. A config has reached its lease
    */
    if (need_new_score ||
        configs[scores[live_slot].config_index].state == VW::reductions::automl::config_state::Removed ||
        scores[live_slot].update_count >= configs[scores[live_slot].config_index].lease)
    {
      // Double the lease check swap for eligible_to_inactivate configs
      if (!need_new_score &&
          configs[scores[live_slot].config_index].state == VW::reductions::automl::config_state::Live)
      {
        configs[scores[live_slot].config_index].lease *= 2;
        if (!scores[live_slot].eligible_to_inactivate || swap_eligible_to_inactivate(live_slot)) { continue; }
      }
      // Skip over removed configs in index queue, and do nothing we we run out of eligible configs
      while (!index_queue.empty() &&
          configs[index_queue.top().second].state == VW::reductions::automl::config_state::Removed)
      { index_queue.pop(); }
      if (index_queue.empty() && !repopulate_index_queue()) { continue; }
      // Allocate new score if we haven't reached maximum yet
      if (need_new_score)
      {
        scores.emplace_back(automl_significance_level, automl_estimator_decay);
        champ_scores.emplace_back(automl_significance_level, automl_estimator_decay);
        if (live_slot > priority_challengers) { scores.back().eligible_to_inactivate = true; }
      }
      // Only inactivate current config if lease is reached
      if (!need_new_score &&
          configs[scores[live_slot].config_index].state == VW::reductions::automl::config_state::Live)
      { configs[scores[live_slot].config_index].state = VW::reductions::automl::config_state::Inactive; }
      // Set all features of new live config
      scores[live_slot].reset_stats(automl_significance_level, automl_estimator_decay);
      champ_scores[live_slot].reset_stats(automl_significance_level, automl_estimator_decay);
      uint64_t new_live_config_index = choose();
      scores[live_slot].config_index = new_live_config_index;
      configs[new_live_config_index].state = VW::reductions::automl::config_state::Live;
      weights.copy_offsets(current_champ, live_slot, wpp);
      // Regenerate interactions each time an exclusion is swapped in
      gen_interactions(live_slot);
      // We may also want to 0 out weights here? Currently keep all same in live_slot position
    }
  }
}

bool interaction_config_manager::better(uint64_t live_slot)
{
  return lb_trick ? scores[live_slot].lower_bound() > (1.f - champ_scores[live_slot].lower_bound()) : scores[live_slot].lower_bound() > champ_scores[live_slot].upper_bound();
}

bool interaction_config_manager::worse(uint64_t)
{
  // Dummy return false
  return false;
}

uint64_t interaction_config_manager::choose()
{
  uint64_t ret = index_queue.top().second;
  index_queue.pop();
  return ret;
}

void interaction_config_manager::update_champ()
{
  bool champ_change = false;

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
  {
    if (live_slot == current_champ) { continue; }
    // If challenger is better ('better function from Chacha')
    if (better(live_slot))
    {
      champ_change = true;
      // Update champ with live_slot of live config
      if (scores[live_slot].eligible_to_inactivate)
      {
        scores[live_slot].eligible_to_inactivate = false;
        scores[current_champ].eligible_to_inactivate = true;
      }
      current_champ = live_slot;
    }
    else if (worse(live_slot))  // If challenger is worse ('worse function from Chacha')
    {
      configs[scores[live_slot].config_index].state = VW::reductions::automl::config_state::Removed;
    }
  }
  if (champ_change)
  {
    this->total_champ_switches++;
    if (!keep_configs)
    {
      while (!index_queue.empty()) { index_queue.pop(); };
      uint64_t champ_config_index = scores[current_champ].config_index;
      if (champ_config_index != 0)
      {
        exclusion_config& zero_ind = configs[0];
        exclusion_config& new_champ = configs[champ_config_index];
        std::swap(zero_ind, new_champ);
        scores[current_champ].config_index = 0;
      }
      auto champ_primary_score = std::move(scores[current_champ]);
      auto champ_secondary_score = std::move(champ_scores[current_champ]);
      scores.clear();
      champ_scores.clear();
      scores.push_back(std::move(champ_primary_score));
      champ_scores.push_back(std::move(champ_secondary_score));
      current_champ = 0;
      valid_config_size = 1;
    }
    for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
    {
      scores[live_slot].reset_stats(automl_significance_level, automl_estimator_decay);
      champ_scores[live_slot].reset_stats(automl_significance_level, automl_estimator_decay);
    }
    config_oracle();
  }
}

void interaction_config_manager::persist(metric_sink& metrics, bool verbose)
{
  metrics.set_uint("test_county", total_learn_count);
  metrics.set_uint("current_champ", current_champ);
  for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
  {
    scores[live_slot].persist(metrics, "_" + std::to_string(live_slot), verbose, interaction_type);
    if (verbose)
    {
      auto& exclusions = configs[scores[live_slot].config_index].exclusions;
      metrics.set_string("exclusionc_" + std::to_string(live_slot), ::exclusions_to_string(exclusions));
    }
  }
  metrics.set_uint("total_champ_switches", total_champ_switches);
}

// This sets up example with correct ineractions vector
void interaction_config_manager::apply_config(example* ec, uint64_t live_slot)
{
  if (ec == nullptr) { return; }
  if (live_slot < max_live_configs) { ec->interactions = &(scores[live_slot].live_interactions); }
  else
  {
    THROW("fatal (automl): trying to apply a config higher than max configs allowed");
  }
}

// inner loop of learn driven by # MAX_CONFIGS
template <typename CMType>
void automl<CMType>::offset_learn(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
{
  VW::reductions::automl::interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  // if (ec[0]->interactions != nullptr)
  // { logger->out_info("offlearn: incoming interaction: {}", ::interaction_vec_t_to_string(*(ec[0]->interactions))); }
  const float w = logged.probability > 0 ? 1 / logged.probability : 0;
  const float r = -logged.cost;

  std::swap(ec[0]->pred.a_s, buffer_a_s);

  int64_t current_slot_index = cm->scores.size() - 1;
  int64_t live_slot = current_slot_index;
  int64_t current_champ = static_cast<int64_t>(cm->current_champ);
  assert(current_champ >= 0);

  auto restore_guard = VW::scope_exit([this, &ec, &live_slot, &current_champ, &incoming_interactions]() {
    for (example* ex : ec) { ex->interactions = incoming_interactions; }
    if (live_slot >= 0 && live_slot != current_champ) { std::swap(ec[0]->pred.a_s, buffer_a_s); }
  });

  for (; current_slot_index >= 0; current_slot_index -= 1)
  {
    if (!debug_reverse_learning_order) { live_slot = current_slot_index; }
    else
    {
      live_slot = cm->scores.size() - 1 - current_slot_index;
    }

    if (live_slot == current_champ) { std::swap(ec[0]->pred.a_s, buffer_a_s); }
    else
    {
      ec[0]->pred.a_s.clear();
    }

    for (example* ex : ec) { cm->apply_config(ex, live_slot); }

    if (!base.learn_returns_prediction) { base.predict(ec, live_slot); }
    base.learn(ec, live_slot);
    const uint32_t chosen_action = ec[0]->pred.a_s[0].action;
    if (live_slot == current_champ)
    {
      for (size_t live_slot_upd = 0; live_slot_upd < cm->scores.size(); ++live_slot_upd)
      {
        if (live_slot_upd == static_cast<size_t>(current_champ)) { continue; }
        cm->champ_scores[live_slot_upd].update(chosen_action == labelled_action ? w : 0, r);
      }
      std::swap(ec[0]->pred.a_s, buffer_a_s);
    }
    else
    {
      cm->scores[live_slot].update(chosen_action == labelled_action ? w : 0, r);
    }
  }

  std::swap(ec[0]->pred.a_s, buffer_a_s);
}

}  // namespace automl
}  // namespace reductions
}  // namespace VW

namespace
{
template <typename CMType, bool is_explore>
void predict_automl(VW::reductions::automl::automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
{
  VW::reductions::automl::interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  // if (ec[0]->interactions != nullptr)
  // { data.logger->out_info("pred: incoming interaction: {}", ::interaction_vec_t_to_string(*(ec[0]->interactions))); }
  uint64_t champ_live_slot = data.cm->current_champ;
  for (VW::example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

  auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
    for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
  });

  base.predict(ec, champ_live_slot);
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(VW::reductions::automl::automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
{
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it = std::find_if(ec.begin(), ec.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });

  if (it != ec.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(ec.begin(), it);
  }

  data.one_step(base, ec, logged, labelled_action);
  assert(ec[0]->interactions != nullptr);
}

template <typename CMType, bool verbose>
void persist(VW::reductions::automl::automl<CMType>& data, VW::metric_sink& metrics)
{
  if (verbose) { data.cm->persist(metrics, true); }
  else
  {
    data.cm->persist(metrics, false);
  }
}

template <typename CMType>
void finish_example(VW::workspace& all, VW::reductions::automl::automl<CMType>& data, VW::multi_ex& ec)
{
  VW::reductions::automl::interaction_vec_t* incoming_interactions = ec[0]->interactions;

  uint64_t champ_live_slot = data.cm->current_champ;
  for (VW::example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

  {
    auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
      for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
    });

    data.adf_learner->print_example(all, ec);
  }

  VW::finish_example(all, ec);
}

template <typename CMType>
void save_load_aml(VW::reductions::automl::automl<CMType>& aml, io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, aml); }
  else
  {
    VW::model_utils::write_model_field(io, aml, "_automl", text);
  }
}

// Basic implementation of scheduler to pick new configs when one runs out of lease.
// Highest priority will be picked first because of max-PQ implementation, this will
// be the config with the least exclusion. Note that all configs will run to lease
// before priorities and lease are reset.
float calc_priority_favor_popular_namespaces(
    const VW::reductions::automl::exclusion_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : config.exclusions) { priority -= ns_counter.at(*ns_pair.begin()); }
  return priority;
}

// Same as above, returns 0 (includes rest to remove unused variable warning)
float calc_priority_empty(
    const VW::reductions::automl::exclusion_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  _UNUSED(config);
  _UNUSED(ns_counter);
  return 0.f;
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  uint64_t global_lease;
  uint64_t max_live_configs;
  std::string cm_type;
  std::string priority_type;
  int32_t priority_challengers;
  bool keep_configs = false;
  bool verbose_metrics = false;
  std::string interaction_type;
  std::string oracle_type;
  float automl_significance_level;
  float automl_estimator_decay;
  bool reversed_learning_order = false;
  bool lb_trick = false;

  option_group_definition new_options("[Reduction] Automl");
  new_options
      .add(make_option("automl", max_live_configs)
               .necessary()
               .keep()
               .default_value(3)
               .help("Set number of live configs")
               .experimental())
      .add(make_option("global_lease", global_lease)
               .keep()
               .default_value(10)
               .help("Set initial lease for automl interactions")
               .experimental())
      .add(make_option("cm_type", cm_type)
               .keep()
               .default_value("interaction")
               .one_of({"interaction"})
               .help("Set type of config manager")
               .experimental())
      .add(make_option("priority_type", priority_type)
               .keep()
               .default_value("none")
               .one_of({"none", "favor_popular_namespaces"})
               .help("Set function to determine next config")
               .experimental())
      .add(make_option("priority_challengers", priority_challengers)
               .keep()
               .default_value(-1)
               .help("Set number of priority challengers to use")
               .experimental())
      .add(make_option("keep_configs", keep_configs).keep().help("Keep all configs after champ change").experimental())
      .add(make_option("verbose_metrics", verbose_metrics).help("Extended metrics for debugging").experimental())
      .add(make_option("interaction_type", interaction_type)
               .keep()
               .default_value("quadratic")
               .one_of({"quadratic", "cubic"})
               .help("Set what type of interactions to use")
               .experimental())
      .add(make_option("oracle_type", oracle_type)
               .keep()
               .default_value("one_diff")
               .one_of({"one_diff", "rand", "champdupe"})
               .help("Set oracle to generate configs")
               .experimental())
      .add(make_option("debug_reversed_learn", reversed_learning_order)
               .default_value(false)
               .help("Debug: learn each config in reversed order (last to first).")
               .experimental())
      .add(make_option("lb_trick", lb_trick)
               .default_value(false)
               .help("Use 1-lower_bound as upper_bound for estimator")
               .experimental())
      .add(make_option("automl_significance_level", automl_significance_level)
               .keep()
               .default_value(DEFAULT_ALPHA)
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("automl_estimator_decay", automl_estimator_decay)
               .keep()
               .default_value(CRESSEREAD_DEFAULT_TAU)
               .help("Time constant for count decay")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  VW::reductions::automl::priority_func* calc_priority;

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "favor_popular_namespaces")
  {
    calc_priority = &calc_priority_favor_popular_namespaces;
  }
  else
  {
    THROW("Invalid priority function provided");
  }

  if (priority_challengers < 0) { priority_challengers = (static_cast<int>(max_live_configs) - 1) / 2; }

  // Note that all.wpp will not be set correctly until after setup
  auto cm = VW::make_unique<VW::reductions::automl::interaction_config_manager>(global_lease, max_live_configs,
      all.get_random_state(), static_cast<uint64_t>(priority_challengers), keep_configs, interaction_type, oracle_type,
      all.weights.dense_weights, calc_priority, automl_significance_level, automl_estimator_decay, &all.logger,
      all.wpp);
  auto data = VW::make_unique<VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager>>(
      std::move(cm), &all.logger);
  data->debug_reverse_learning_order = reversed_learning_order;
  data->cm->lb_trick = lb_trick;
  if (max_live_configs > MAX_CONFIGS)
  {
    THROW("Maximum number of configs is "
        << MAX_CONFIGS << " and " << max_live_configs
        << " were specified. Please decrease the number of configs with the --automl flag.");
  }

  // override and clear all the global interactions
  // see parser.cc line 740
  all.interactions.clear();
  assert(all.interactions.empty() == true);

  // make sure we setup the rest of the stack with cleared interactions
  // to make sure there are not subtle bugs
  auto* base_learner = stack_builder.setup_base_learner();

  assert(all.interactions.empty() == true);

  assert(all.weights.sparse == false);
  if (all.weights.sparse) THROW("--automl does not work with sparse weights");

  ::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  // only this has been tested
  if (base_learner->is_multiline())
  {
    auto ppw = max_live_configs;
    auto* persist_ptr = verbose_metrics ? persist<VW::reductions::automl::interaction_config_manager, true>
                                        : persist<VW::reductions::automl::interaction_config_manager, false>;
    data->adf_learner = as_multiline(base_learner->get_learner_by_name_prefix("cb_adf"));
    auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner),
        learn_automl<VW::reductions::automl::interaction_config_manager, true>,
        predict_automl<VW::reductions::automl::interaction_config_manager, true>,
        stack_builder.get_setupfn_name(automl_setup))
                  .set_params_per_weight(ppw)  // refactor pm
                  .set_output_prediction_type(VW::prediction_type_t::action_scores)
                  .set_input_label_type(VW::label_type_t::cb)
                  .set_input_prediction_type(VW::prediction_type_t::action_scores)
                  .set_output_label_type(VW::label_type_t::cb)
                  .set_finish_example(::finish_example<VW::reductions::automl::interaction_config_manager>)
                  .set_save_load(save_load_aml<VW::reductions::automl::interaction_config_manager>)
                  .set_persist_metrics(persist_ptr)
                  .set_output_prediction_type(base_learner->get_output_prediction_type())
                  .set_learn_returns_prediction(true)
                  .build();
    return make_base(*l);
  }
  else
  {
    // not implemented yet
    THROW("fatal: automl not supported for single line learners yet");
  }
}

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, VW::reductions::automl::exclusion_config& ec)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ec.exclusions);
  bytes += read_model_field(io, ec.lease);
  bytes += read_model_field(io, ec.state);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::reductions::automl::exclusion_config& ec, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ec.exclusions, upstream_name + "_exclusions", text);
  bytes += write_model_field(io, ec.lease, upstream_name + "_lease", text);
  bytes += write_model_field(io, ec.state, upstream_name + "_state", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::automl::aml_score& amls)
{
  size_t bytes = 0;
  bytes += read_model_field(io, reinterpret_cast<VW::scored_config&>(amls));
  bytes += read_model_field(io, amls.config_index);
  bytes += read_model_field(io, amls.eligible_to_inactivate);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::reductions::automl::aml_score& amls, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, reinterpret_cast<const VW::scored_config&>(amls), upstream_name, text);
  bytes += write_model_field(io, amls.config_index, upstream_name + "_index", text);
  bytes += write_model_field(io, amls.eligible_to_inactivate, upstream_name + "_eligible_to_inactivate", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::automl::interaction_config_manager& cm)
{
  cm.scores.clear();
  size_t bytes = 0;
  bytes += read_model_field(io, cm.total_learn_count);
  bytes += read_model_field(io, cm.current_champ);
  bytes += read_model_field(io, cm.valid_config_size);
  bytes += read_model_field(io, cm.ns_counter);
  bytes += read_model_field(io, cm.configs);
  bytes += read_model_field(io, cm.scores);
  bytes += read_model_field(io, cm.champ_scores);
  bytes += read_model_field(io, cm.index_queue);
  for (uint64_t live_slot = 0; live_slot < cm.scores.size(); ++live_slot) { cm.gen_interactions(live_slot); }
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::reductions::automl::interaction_config_manager& cm,
    const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cm.total_learn_count, upstream_name + "_count", text);
  bytes += write_model_field(io, cm.current_champ, upstream_name + "_champ", text);
  bytes += write_model_field(io, cm.valid_config_size, upstream_name + "_valid_config_size", text);
  bytes += write_model_field(io, cm.ns_counter, upstream_name + "_ns_counter", text);
  bytes += write_model_field(io, cm.configs, upstream_name + "_configs", text);
  bytes += write_model_field(io, cm.scores, upstream_name + "_scores", text);
  bytes += write_model_field(io, cm.champ_scores, upstream_name + "_champ_scores", text);
  bytes += write_model_field(io, cm.index_queue, upstream_name + "_index_queue", text);
  return bytes;
}

template <typename CMType>
size_t read_model_field(io_buf& io, VW::reductions::automl::automl<CMType>& aml)
{
  size_t bytes = 0;
  bytes += read_model_field(io, aml.current_state);
  bytes += read_model_field(io, *aml.cm);
  return bytes;
}

template <typename CMType>
size_t write_model_field(
    io_buf& io, const VW::reductions::automl::automl<CMType>& aml, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, aml.current_state, upstream_name + "_state", text);
  bytes += write_model_field(io, *aml.cm, upstream_name + "_config_manager", text);
  return bytes;
}

}  // namespace model_utils

VW::string_view to_string(reductions::automl::automl_state state)
{
  switch (state)
  {
    case reductions::automl::automl_state::Collecting:
      return "Collecting";
    case reductions::automl::automl_state::Experimenting:
      return "Experimenting";
  }

  assert(false);
  return "unknown";
}

VW::string_view to_string(reductions::automl::config_state state)
{
  switch (state)
  {
    case reductions::automl::config_state::New:
      return "New";
    case reductions::automl::config_state::Live:
      return "Live";
    case reductions::automl::config_state::Inactive:
      return "Inactive";
    case reductions::automl::config_state::Removed:
      return "Removed";
  }

  assert(false);
  return "unknown";
}
}  // namespace VW
