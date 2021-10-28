// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "automl.h"

#include "constant.h"  // NUM_NAMESPACES
#include "debug_log.h"
#include "io/logger.h"
#include "vw.h"
#include "model_utils.h"

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;

namespace logger = VW::io::logger;

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

namespace VW
{
namespace automl
{
namespace details
{
// fail if incompatible reductions got setup
// todo: audit if they reference global all interactions
void fail_if_enabled(vw& all, const std::set<std::string>& not_compat)
{
  std::vector<std::string> enabled_reductions;
  if (all.l != nullptr) all.l->get_enabled_reductions(enabled_reductions);

  for (const auto& reduction : enabled_reductions)
  {
    if (not_compat.count(reduction) > 0) THROW("Error: automl does not yet support this reduction: " + reduction);
  }
}

/*
void print_weights_nonzero(vw* all, uint64_t count, dense_parameters& weights)
{
  for (auto it = weights.begin(); it != weights.end(); ++it)
  {
    assert(weights.stride_shift() == 2);
    auto real_index = it.index() >> weights.stride_shift();
    // if (MAX_CONFIGS > 4)
    //   assert(all->wpp == 8);
    // else
    //   assert(all->wpp == 4);

    int type = real_index & (all->wpp - 1);

    uint64_t off = 0;
    auto zero = (&(*it))[0 + off];
    if (!cmpf(zero, 0.f))
    {
      if (type == 0) { std::cerr << (real_index) << ":c" << count << ":0:" << zero << std::endl; }
      else if (type == 1)
      {
        std::cerr << (real_index - 1) << ":c" << count << ":1:" << zero << std::endl;
      }
      else if (type == 2)
      {
        std::cerr << (real_index - 2) << ":c" << count << ":2:" << zero << std::endl;
      }
      else if (type == 3)
      {
        std::cerr << (real_index - 3) << ":c" << count << ":3:" << zero << std::endl;
      }
      else if (type == 4)
      {
        std::cerr << (real_index - 4) << ":c" << count << ":4:" << zero << std::endl;
      }
      else if (type == 5)
      {
        std::cerr << (real_index - 5) << ":c" << count << ":5:" << zero << std::endl;
      }
    }
  }
  std::cerr << std::endl;
}
*/
}  // namespace details

void scored_config::update(float w, float r)
{
  update_count++;
  chisq.update(w, r);
  ips += r * w;
  last_w = w;
  last_r = r;
}

void scored_config::persist(metric_sink& metrics, const std::string& suffix)
{
  metrics.set_uint("upcnt" + suffix, update_count);
  metrics.set_float("ips" + suffix, current_ips());
  distributionally_robust::ScoredDual sd = chisq.recompute_duals();
  metrics.set_float("bound" + suffix, static_cast<float>(sd.first));
  metrics.set_float("w" + suffix, last_w);
  metrics.set_float("r" + suffix, last_r);
  metrics.set_uint("conf_idx" + suffix, config_index);
}

float scored_config::current_ips() const { return (update_count > 0) ? ips / update_count : 0; }

void scored_config::reset_stats()
{
  chisq.reset(0.05, 0.999);
  ips = 0.0;
  last_w = 0.0;
  last_r = 0.0;
  update_count = 0;
}

template <typename CMType>
void automl<CMType>::one_step(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
{
  cm->total_learn_count++;
  switch (current_state)
  {
    case VW::automl::automl_state::Collecting:
      cm->pre_process(ec);
      cm->config_oracle();
      offset_learn(base, ec, logged, labelled_action);
      current_state = VW::automl::automl_state::Experimenting;
      break;

    case VW::automl::automl_state::Experimenting:
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
interaction_config_manager::interaction_config_manager(uint64_t global_lease, uint64_t max_live_configs, uint64_t seed,
    uint64_t priority_challengers, priority_func* calc_priority)
    : global_lease(global_lease)
    , max_live_configs(max_live_configs)
    , seed(seed)
    , priority_challengers(priority_challengers)
    , calc_priority(calc_priority)
{
  random_state.set_random_state(seed);
  exclusion_config conf(global_lease);
  conf.state = VW::automl::config_state::Live;
  configs[0] = conf;
  scored_config sc;
  scores.push_back(sc);
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding live_slot. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.
void interaction_config_manager::gen_quadratic_interactions(uint64_t live_slot)
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
      if (exclusions.find(idx1) == exclusions.end() || exclusions.at(idx1).find(idx2) == exclusions.at(idx1).end())
      { interactions.push_back({idx1, idx2}); }
    }
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
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_ns_seen = true; }
    }
  }

  // Regenerate interactions if new namespaces are seen
  if (new_ns_seen)
  {
    for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot) { gen_quadratic_interactions(live_slot); }
  }
}

// This will generate configs based on the current champ. These configs will be
// stored as 'exclusions.' The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
// TODO: Add logic to avoid duplicate configs (could be very costly)
void interaction_config_manager::config_oracle()
{
  auto& champ_interactions = scores[current_champ].live_interactions;
  for (uint64_t i = 0; i < CONGIGS_PER_CHAMP_CHANGE; ++i)
  {
    uint64_t rand_ind = static_cast<uint64_t>(random_state.get_and_update_random() * champ_interactions.size());
    namespace_index ns1 = champ_interactions[rand_ind][0];
    namespace_index ns2 = champ_interactions[rand_ind][1];
    std::map<namespace_index, std::set<namespace_index>> new_exclusions(
        configs[scores[current_champ].config_index].exclusions);
    new_exclusions[ns1].insert(ns2);
    uint64_t config_index = configs.size();
    exclusion_config conf(global_lease);
    conf.exclusions = new_exclusions;
    configs[config_index] = conf;
    float priority = (*calc_priority)(configs[config_index], ns_counter);
    index_queue.push(std::make_pair(priority, config_index));
  }
}

// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their lease. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of lease. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
bool interaction_config_manager::repopulate_index_queue()
{
  for (const auto& ind_config : configs)
  {
    uint64_t redo_index = ind_config.first;
    // Only re-add if not removed and not live
    if (configs[redo_index].state == VW::automl::config_state::New ||
        configs[redo_index].state == VW::automl::config_state::Inactive)
    {
      float priority = (*calc_priority)(configs[redo_index], ns_counter);
      index_queue.push(std::make_pair(priority, redo_index));
    }
  }
  return !index_queue.empty();
}

bool interaction_config_manager::swap_eligible_to_inactivate(uint64_t live_slot)
{
  for (uint64_t other_live_slot = 0; other_live_slot < scores.size(); ++other_live_slot)
  {
    if (!scores[other_live_slot].eligible_to_inactivate && other_live_slot != current_champ &&
        better(configs[scores[live_slot].config_index], configs[scores[other_live_slot].config_index]))
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
    if (need_new_score || configs[scores[live_slot].config_index].state == VW::automl::config_state::Removed ||
        scores[live_slot].update_count >= configs[scores[live_slot].config_index].lease)
    {
      // Double the lease check swap for eligible_to_inactivate configs
      if (!need_new_score && configs[scores[live_slot].config_index].state == VW::automl::config_state::Live)
      {
        configs[scores[live_slot].config_index].lease *= 2;
        if (!scores[live_slot].eligible_to_inactivate || swap_eligible_to_inactivate(live_slot)) { continue; }
      }
      // Skip over removed configs in index queue, and do nothing we we run out of eligible configs
      while (!index_queue.empty() && configs[index_queue.top().second].state == VW::automl::config_state::Removed)
      { index_queue.pop(); }
      if (index_queue.empty() && !repopulate_index_queue()) { continue; }
      // Allocate new score if we haven't reached maximum yet
      if (need_new_score)
      {
        scored_config sc;
        if (live_slot > priority_challengers) { sc.eligible_to_inactivate = true; }
        scores.push_back(sc);
      }
      // Only inactivate current config if lease is reached
      if (!need_new_score && configs[scores[live_slot].config_index].state == VW::automl::config_state::Live)
      { configs[scores[live_slot].config_index].state = VW::automl::config_state::Inactive; }
      // Set all features of new live config
      scores[live_slot].reset_stats();
      uint64_t new_live_config_index = choose();
      scores[live_slot].config_index = new_live_config_index;
      configs[new_live_config_index].state = VW::automl::config_state::Live;
      // Regenerate interactions each time an exclusion is swapped in
      gen_quadratic_interactions(live_slot);
      // We may also want to 0 out weights here? Currently keep all same in live_slot position
    }
  }
}

bool interaction_config_manager::better(const exclusion_config& challenger, const exclusion_config& champ) const
{
  return challenger.lower_bound > champ.ips;
}

bool interaction_config_manager::worse(const exclusion_config& challenger, const exclusion_config& champ) const
{
  // Dummy return false to remove unused variable warning
  return (champ.lower_bound > challenger.ips) ? false : false;
}

uint64_t interaction_config_manager::choose()
{
  uint64_t ret = index_queue.top().second;
  index_queue.pop();
  return ret;
}

void interaction_config_manager::update_champ()
{
  // Update ips and lower bound for live configs
  for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
  {
    float ips = scores[live_slot].current_ips();
    distributionally_robust::ScoredDual sd = scores[live_slot].chisq.recompute_duals();
    float lower_bound = static_cast<float>(sd.first);
    configs[scores[live_slot].config_index].ips = ips;
    configs[scores[live_slot].config_index].lower_bound = lower_bound;
  }
  exclusion_config champ_config = configs[scores[current_champ].config_index];
  bool champ_change = false;

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (uint64_t config_index = 0; config_index < configs.size(); ++config_index)
  {
    if (configs[config_index].state == VW::automl::config_state::New ||
        configs[config_index].state == VW::automl::config_state::Removed ||
        scores[current_champ].config_index == config_index)
    { continue; }
    // If challenger is better ('better function from Chacha')
    if (better(configs[config_index], champ_config))
    {
      champ_change = true;
      if (configs[config_index].state == VW::automl::config_state::Live)
      {
        // Update champ with live_slot of live config
        for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
        {
          if (scores[live_slot].config_index == config_index)
          {
            if (scores[live_slot].eligible_to_inactivate)
            {
              scores[live_slot].eligible_to_inactivate = false;
              scores[current_champ].eligible_to_inactivate = true;
            }
            current_champ = live_slot;
            break;
          }
        }
        champ_config = configs[config_index];
      }
      else
      {
        float worst_ips = std::numeric_limits<float>::infinity();
        uint64_t worst_live_slot = 0;
        for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
        {
          if (configs[scores[live_slot].config_index].ips < worst_ips)
          {
            worst_ips = configs[scores[live_slot].config_index].ips;
            worst_live_slot = live_slot;
          }
        }
        configs[scores[worst_live_slot].config_index].lease *= 2;
        configs[scores[worst_live_slot].config_index].state = VW::automl::config_state::Inactive;
        scores[worst_live_slot].reset_stats();
        scores[worst_live_slot].config_index = config_index;
        configs[scores[worst_live_slot].config_index].state = VW::automl::config_state::Live;
        gen_quadratic_interactions(worst_live_slot);
        if (scores[worst_live_slot].eligible_to_inactivate)
        {
          scores[worst_live_slot].eligible_to_inactivate = false;
          scores[current_champ].eligible_to_inactivate = true;
        }
        current_champ = worst_live_slot;
        // Rare case breaks index_queue, best to just clear it
        while (!index_queue.empty()) { index_queue.pop(); };
      }
    }
    else if (worse(configs[config_index], champ_config))  // If challenger is worse ('worse function from Chacha')
    {
      configs[config_index].state = VW::automl::config_state::Removed;
    }
  }
  if (champ_change) { config_oracle(); }
}

void interaction_config_manager::persist(metric_sink& metrics)
{
  metrics.set_uint("test_county", total_learn_count);
  metrics.set_uint("current_champ", current_champ);
  for (uint64_t live_slot = 0; live_slot < scores.size(); ++live_slot)
  { scores[live_slot].persist(metrics, "_" + std::to_string(live_slot)); }
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

void interaction_config_manager::revert_config(example* ec) { ec->interactions = nullptr; }

template <typename CMType, bool is_explore>
void predict_automl(automl<CMType>& data, multi_learner& base, multi_ex& ec)
{
  uint64_t champ_live_slot = data.cm->current_champ;
  for (example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

  auto restore_guard = VW::scope_exit([&data, &ec] {
    for (example* ex : ec) { data.cm->revert_config(ex); }
  });

  base.predict(ec, champ_live_slot);
}

// inner loop of learn driven by # MAX_CONFIGS
template <typename CMType>
void automl<CMType>::offset_learn(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
{
  for (uint64_t live_slot = 0; live_slot < cm->scores.size(); ++live_slot)
  {
    for (example* ex : ec) { cm->apply_config(ex, live_slot); }

    auto restore_guard = VW::scope_exit([this, &ec] {
      for (example* ex : ec) { this->cm->revert_config(ex); }
    });

    if (!base.learn_returns_prediction) { base.predict(ec, live_slot); }

    base.learn(ec, live_slot);

    const auto& action_scores = ec[0]->pred.a_s;
    // cb_adf => first action is a greedy action
    const auto maxit = action_scores.begin();
    const uint32_t chosen_action = maxit->action;

    // extra asserts
    assert(chosen_action < ec.size());
    assert(labelled_action < ec.size());

    const float w = logged.probability > 0 ? 1 / logged.probability : 0;
    const float r = -logged.cost;

    cm->scores[live_slot].update(chosen_action == labelled_action ? w : 0, r);

    // cache the champ
    if (cm->current_champ == live_slot) { champ_a_s = std::move(ec[0]->pred.a_s); }
  }
  // replace bc champ always gets cached
  ec[0]->pred.a_s = std::move(champ_a_s);
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(automl<CMType>& data, multi_learner& base, multi_ex& ec)
{
  CB::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it = std::find_if(ec.begin(), ec.end(), [](example* item) { return !item->l.cb.costs.empty(); });

  if (it != ec.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(ec.begin(), it);
  }

  data.one_step(base, ec, logged, labelled_action);
  assert(ec[0]->interactions == nullptr);
}

template <typename CMType>
void persist(automl<CMType>& data, metric_sink& metrics)
{
  data.cm->persist(metrics);
}

template <typename CMType>
void finish_example(vw& all, automl<CMType>& data, multi_ex& ec)
{
  {
    uint64_t champ_live_slot = data.cm->current_champ;
    for (example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

    auto restore_guard = VW::scope_exit([&data, &ec, &champ_live_slot] {
      for (example* ex : ec) { data.cm->revert_config(ex); }
    });

    data.adf_learner->print_example(all, ec);
  }

  VW::finish_example(all, ec);
}

template <typename CMType>
void save_load_aml(automl<CMType>& aml, io_buf& io, bool read, bool text)
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
float calc_priority_least_exclusion(
    const exclusion_config& config, const std::map<namespace_index, uint64_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : config.exclusions) { priority -= ns_counter.at(ns_pair.first); }
  return priority;
}

// Same as above, returns 0 (includes rest to remove unused variable warning)
float calc_priority_empty(const exclusion_config& config, const std::map<namespace_index, uint64_t>& ns_counter)
{
  _UNUSED(config);
  _UNUSED(ns_counter);
  return 0.f;
}

VW::LEARNER::base_learner* automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();

  uint64_t global_lease;
  uint64_t max_live_configs;
  std::string cm_type;
  std::string priority_type;
  int priority_challengers;

  option_group_definition new_options("Debug: automl reduction");
  new_options
      .add(make_option("automl", max_live_configs)
               .necessary()
               .keep()
               .default_value(3)
               .help("set number of live configs"))
      .add(make_option("global_lease", global_lease)
               .keep()
               .default_value(10)
               .help("set initial lease for automl interactions"))
      .add(make_option("cm_type", cm_type).keep().default_value("interaction").help("set type of config manager"))
      .add(make_option("priority_type", priority_type)
               .keep()
               .default_value("none")
               .help("set function to determine next config {none, least_exclusion}"))
      .add(make_option("priority_challengers", priority_challengers)
               .keep()
               .default_value(-1)
               .help("set number of priority challengers to use"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  priority_func* calc_priority;

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "least_exclusion")
  {
    calc_priority = &calc_priority_least_exclusion;
  }
  else
  {
    THROW("Error: Invalid priority function provided");
  }

  if (priority_challengers < 0) { priority_challengers = (static_cast<int>(max_live_configs) - 1) / 2; }

  auto cm = VW::make_unique<interaction_config_manager>(
      global_lease, max_live_configs, all.random_seed, static_cast<uint64_t>(priority_challengers), calc_priority);
  auto data = VW::make_unique<automl<interaction_config_manager>>(std::move(cm));
  assert(max_live_configs <= MAX_CONFIGS);

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

  details::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  // only this has been tested
  if (base_learner->is_multiline())
  {
    // fetch cb_explore_adf to call directly into the print routine twice
    data->adf_learner = as_multiline(base_learner->get_learner_by_name_prefix("cb_explore_adf_"));
    auto ppw = max_live_configs;

    auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner),
        learn_automl<interaction_config_manager, true>, predict_automl<interaction_config_manager, true>,
        stack_builder.get_setupfn_name(automl_setup))
                  .set_params_per_weight(ppw)  // refactor pm
                  .set_finish_example(finish_example<interaction_config_manager>)
                  .set_save_load(save_load_aml<interaction_config_manager>)
                  .set_persist_metrics(persist<interaction_config_manager>)
                  .set_prediction_type(base_learner->get_output_prediction_type())
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
}  // namespace automl

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::automl::exclusion_config& ec)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ec.exclusions);
  bytes += read_model_field(io, ec.lease);
  bytes += read_model_field(io, ec.ips);
  bytes += read_model_field(io, ec.lower_bound);
  bytes += read_model_field(io, ec.state);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::automl::exclusion_config& ec, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ec.exclusions, upstream_name + "_exclusions", text);
  bytes += write_model_field(io, ec.lease, upstream_name + "_lease", text);
  bytes += write_model_field(io, ec.ips, upstream_name + "_ips", text);
  bytes += write_model_field(io, ec.lower_bound, upstream_name + "_lower_bound", text);
  bytes += write_model_field(io, ec.state, upstream_name + "_state", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::automl::scored_config& sc)
{
  size_t bytes = 0;
  bytes += read_model_field(io, sc.ips);
  bytes += read_model_field(io, sc.update_count);
  bytes += read_model_field(io, sc.last_w);
  bytes += read_model_field(io, sc.last_r);
  bytes += read_model_field(io, sc.config_index);
  bytes += read_model_field(io, sc.eligible_to_inactivate);
  bytes += read_model_field(io, sc.chisq);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::automl::scored_config& sc, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, sc.ips, upstream_name + "_ips", text);
  bytes += write_model_field(io, sc.update_count, upstream_name + "_count", text);
  bytes += write_model_field(io, sc.last_w, upstream_name + "_lastw", text);
  bytes += write_model_field(io, sc.last_r, upstream_name + "_lastr", text);
  bytes += write_model_field(io, sc.config_index, upstream_name + "_index", text);
  bytes += write_model_field(io, sc.eligible_to_inactivate, upstream_name + "_eligible_to_inactivate", text);
  bytes += write_model_field(io, sc.chisq, upstream_name + "_chisq", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::automl::interaction_config_manager& cm)
{
  cm.scores.clear();
  size_t bytes = 0;
  bytes += read_model_field(io, cm.total_learn_count);
  bytes += read_model_field(io, cm.current_champ);
  bytes += read_model_field(io, cm.ns_counter);
  bytes += read_model_field(io, cm.configs);
  bytes += read_model_field(io, cm.scores);
  bytes += read_model_field(io, cm.index_queue);
  for (uint64_t live_slot = 0; live_slot < cm.scores.size(); ++live_slot) { cm.gen_quadratic_interactions(live_slot); }
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::automl::interaction_config_manager& cm, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, cm.total_learn_count, upstream_name + "_count", text);
  bytes += write_model_field(io, cm.current_champ, upstream_name + "_champ", text);
  bytes += write_model_field(io, cm.ns_counter, upstream_name + "_ns_counter", text);
  bytes += write_model_field(io, cm.configs, upstream_name + "_configs", text);
  bytes += write_model_field(io, cm.scores, upstream_name + "_scores", text);
  bytes += write_model_field(io, cm.index_queue, upstream_name + "_index_queue", text);
  return bytes;
}

template <typename CMType>
size_t read_model_field(io_buf& io, VW::automl::automl<CMType>& aml)
{
  size_t bytes = 0;
  bytes += read_model_field(io, aml.current_state);
  bytes += read_model_field(io, *aml.cm);
  return bytes;
}

template <typename CMType>
size_t write_model_field(io_buf& io, const VW::automl::automl<CMType>& aml, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, aml.current_state, upstream_name + "_state", text);
  bytes += write_model_field(io, *aml.cm, upstream_name + "_config_manager", text);
  return bytes;
}

}  // namespace model_utils
}  // namespace VW
