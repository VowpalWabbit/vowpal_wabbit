// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "automl.h"
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include "io/logger.h"
#include "vw.h"

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
ConfigOracle -> exclusion_configs_oracle(): Generates new configs based on champ
Schedule -> schedule(): Swaps out / manages live configs on each step
Schedule.median -> better_than_median(): Checks if config is better than median
Schedule.Choose -> calc_priority(): Determines priority when selecting next live config
Predict/Incur Loss -> offset_learn(): Updates weights and learns on live configs
Better -> update_champ()(section noted in code): Changes champ if challenger is better
Worse -> update_champ()(section noted in code): Removes challenger is much worse than champ
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

  for (auto reduction : enabled_reductions)
  {
    if (not_compat.count(reduction) > 0) THROW("automl.cc: plz no bad stack" + reduction);
  }
}

/*
void print_weights_nonzero(vw* all, size_t count, dense_parameters& weights)
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

    size_t off = 0;
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

void scored_config::save_load_scored_config(io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream msg;
  if (!read) msg << "_aml_config_ips " << ips << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&ips), sizeof(ips), read, msg, text);

  if (!read) msg << "_aml_config_count " << update_count << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&update_count), sizeof(update_count), read, msg, text);

  if (!read) msg << "_aml_config_lastw " << last_w << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&last_w), sizeof(last_w), read, msg, text);

  if (!read) msg << "_aml_config_lastr " << last_r << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&last_r), sizeof(last_r), read, msg, text);

  if (!read) msg << "_aml_config_index " << config_index << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&config_index), sizeof(config_index), read, msg, text);

  chisq.save_load(model_file, read, text);
}

void scored_config::persist(metric_sink& metrics, const std::string& suffix)
{
  metrics.int_metrics_list.emplace_back("upcnt" + suffix, update_count);
  metrics.float_metrics_list.emplace_back("ips" + suffix, current_ips());
  distributionally_robust::ScoredDual sd = chisq.recompute_duals();
  metrics.float_metrics_list.emplace_back("bound" + suffix, static_cast<float>(sd.first));
  metrics.float_metrics_list.emplace_back("w" + suffix, last_w);
  metrics.float_metrics_list.emplace_back("r" + suffix, last_r);
  metrics.int_metrics_list.emplace_back("conf_idx" + suffix, config_index);
}

float scored_config::current_ips() const { return ips / update_count; }

void scored_config::reset_stats()
{
  chisq.reset(0.05, 0.999);
  ips = 0.0;
  last_w = 0.0;
  last_r = 0.0;
  update_count = 0;
}

void exclusion_config::save_load_exclusion_config(io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream msg;

  size_t exclusion_size;
  if (read)
  {
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&exclusion_size), sizeof(exclusion_size), read, msg, text);
    for (size_t i = 0; i < exclusion_size; ++i)
    {
      namespace_index ns;
      size_t other_ns_size;
      std::set<namespace_index> other_namespaces;
      bin_text_read_write_fixed(model_file, (char*)&ns, sizeof(ns), read, msg, text);
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&other_ns_size), sizeof(other_ns_size), read, msg, text);
      for (size_t i = 0; i < other_ns_size; ++i)
      {
        namespace_index other_ns;
        bin_text_read_write_fixed(model_file, (char*)&other_ns, sizeof(other_ns), read, msg, text);
        other_namespaces.insert(other_ns);
      }
      exclusions[ns] = other_namespaces;
    }
  }
  else
  {
    exclusion_size = exclusions.size();
    msg << "exclusion_size " << exclusion_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&exclusion_size), sizeof(exclusion_size), read, msg, text);
    for (const auto& ns_pair : exclusions)
    {
      namespace_index ns = ns_pair.first;
      size_t other_ns_size = ns_pair.second.size();
      bin_text_read_write_fixed(model_file, (char*)&ns, sizeof(ns), read, msg, text);
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&other_ns_size), sizeof(other_ns_size), read, msg, text);
      for (const auto& other_ns : ns_pair.second)
      { bin_text_read_write_fixed(model_file, (char*)&other_ns, sizeof(other_ns), read, msg, text); }
    }
  }

  if (!read) msg << "_aml_lease " << lease << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&lease), sizeof(lease), read, msg, text);

  if (!read) msg << "_aml_ips " << ips << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&ips), sizeof(ips), read, msg, text);

  if (!read) msg << "_aml_lower_bound " << lower_bound << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&lower_bound), sizeof(lower_bound), read, msg, text);

  if (!read) msg << "_aml_state " << state << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&state), sizeof(state), read, msg, text);
}

// config_manager is a state machine (config_manager_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own base_learner.learn(). see learn_automl(...)
interaction_config_manager::interaction_config_manager(size_t lease, size_t max_live_configs, uint64_t seed,
    float (*calc_priority)(size_t, const std::map<size_t, exclusion_config>&, const std::map<namespace_index, size_t>&))
    : lease(lease), max_live_configs(max_live_configs), seed(seed), calc_priority(calc_priority)
{
  random_state.set_random_state(seed);
  exclusion_config conf(lease);
  conf.state = Live;
  configs[0] = conf;
  scored_config sc;
  scores.push_back(sc);
}

void interaction_config_manager::one_step(const multi_ex& ec)
{
  switch (current_state)
  {
    case Idle:
      process_namespaces(ec);
      exclusion_configs_oracle();
      current_state = Experimenting;
      break;

    case Collecting:
      break;

    case Experimenting:
      process_namespaces(ec);
      schedule();
      update_champ();
      break;

    default:
      break;
  }
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding stride. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.
void interaction_config_manager::gen_quadratic_interactions(size_t stride)
{
  auto& exclusions = configs[scores[stride].config_index].exclusions;
  auto& interactions = scores[stride].live_interactions;
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
void interaction_config_manager::process_namespaces(const multi_ex& ecs)
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
    for (size_t stride = 0; stride < scores.size(); ++stride) { gen_quadratic_interactions(stride); }
  }
}

// This will generate configs based on the current champ. These configs will be
// stored as 'exclusions.' The current design is to look at the interactions of
// the current champ and remove one interaction for each new config. The number
// of configs to generate per champ is hard-coded to 5 at the moment.
// TODO: Add logic to avoid duplicate configs (could be very costly)
void interaction_config_manager::exclusion_configs_oracle()
{
  auto& champ_interactions = scores[current_champ].live_interactions;
  for (size_t i = 0; i < CONGIGS_PER_CHAMP_CHANGE; ++i)
  {
    size_t rand_ind = static_cast<size_t>(random_state.get_and_update_random() * champ_interactions.size());
    namespace_index ns1 = champ_interactions[rand_ind][0];
    namespace_index ns2 = champ_interactions[rand_ind][1];
    std::map<namespace_index, std::set<namespace_index>> new_exclusions(
        configs[scores[current_champ].config_index].exclusions);
    new_exclusions[ns1].insert(ns2);
    size_t config_index = configs.size();
    exclusion_config conf(lease);
    conf.exclusions = new_exclusions;
    configs[config_index] = conf;
    float priority = (*calc_priority)(config_index, configs, ns_counter);
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
    size_t redo_index = ind_config.first;
    // Only re-add if not removed and not live
    if (configs[redo_index].state == New || configs[redo_index].state == Inactive)
    {
      float priority = (*calc_priority)(redo_index, configs, ns_counter);
      index_queue.push(std::make_pair(priority, redo_index));
    }
  }
  return !index_queue.empty();
}

// Function from Chacha to determine if loss is better than median. Very strict
// requires ips is strictly greater, and strict median
bool interaction_config_manager::better_than_median(size_t stride) const
{
  size_t lower = 0;
  size_t higher = 0;
  for (size_t other_stride = 0; other_stride < scores.size(); ++other_stride)
  {
    if (other_stride != stride)
    {
      if (configs.at(scores[stride].config_index).ips > configs.at(scores[other_stride].config_index).ips) { higher++; }
      else
      {
        lower++;
      }
    }
  }
  return higher > lower;
}

// This function defines the logic update the live configs' leases, and to swap out
// new configs when the lease runs out.
void interaction_config_manager::schedule()
{
  for (size_t stride = 0; stride < max_live_configs; ++stride)
  {
    if (stride == current_champ) { continue; }
    bool need_new_score = scores.size() <= stride;
    /*
    Scheduling a new live config is necessary in 3 cases:
    1. We have not reached the maximum number of live configs yet
    2. The current live config has been removed due to Chacha's worse function
    3. A config has reached its lease
    */
    if (need_new_score || configs[scores[stride].config_index].state == Removed ||
        scores[stride].update_count >= configs[scores[stride].config_index].lease)
    {
      // Double the lease and check median for Live configs
      if (!need_new_score && configs[scores[stride].config_index].state == Live)
      {
        configs[scores[stride].config_index].lease *= 2;
        if (better_than_median(stride)) { return; }
      }
      // Skip over removed configs in index queue, and do nothing we we run out of eligible configs
      while (!index_queue.empty() && configs[index_queue.top().second].state == Removed) { index_queue.pop(); }
      if (index_queue.empty() && !repopulate_index_queue()) { return; }
      // Allocate new score if we haven't reached maximum yet
      if (need_new_score)
      {
        scored_config sc;
        scores.push_back(sc);
      }
      // Only inactivate current config if lease is reached
      if (!need_new_score && configs[scores[stride].config_index].state == Live)
      { configs[scores[stride].config_index].state = Inactive; }
      // Set all features of new live config
      scores[stride].reset_stats();
      size_t new_live_config_index = index_queue.top().second;
      index_queue.pop();
      scores[stride].config_index = new_live_config_index;
      configs[new_live_config_index].state = Live;
      // Regenerate interactions each time an exclusion is swapped in
      gen_quadratic_interactions(stride);
      // We may also want to 0 out weights here? Currently keep all same in stride position
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

void interaction_config_manager::update_champ()
{
  // Update ips and lower bound for live configs
  for (size_t stride = 0; stride < scores.size(); ++stride)
  {
    float ips = scores[stride].current_ips();
    distributionally_robust::ScoredDual sd = scores[stride].chisq.recompute_duals();
    float lower_bound = static_cast<float>(sd.first);
    configs[scores[stride].config_index].ips = ips;
    configs[scores[stride].config_index].lower_bound = lower_bound;
  }
  exclusion_config champ_config = configs[scores[current_champ].config_index];
  bool champ_change = false;

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (size_t config_index = 0; config_index < configs.size(); ++config_index)
  {
    if (configs[config_index].state == New || configs[config_index].state == Removed ||
        scores[current_champ].config_index == config_index)
    { continue; }
    // If challenger is better ('better function from Chacha')
    if (better(configs[config_index], champ_config))
    {
      champ_change = true;
      if (configs[config_index].state == Live)
      {
        // Update champ with stride of live config
        for (size_t stride = 0; stride < scores.size(); ++stride)
        {
          if (scores[stride].config_index == config_index)
          {
            current_champ = stride;
            break;
          }
        }
        champ_config = configs[config_index];
      }
      else
      {
        float worst_ips = std::numeric_limits<float>::infinity();
        size_t worst_stride = 0;
        for (size_t stride = 0; stride < scores.size(); ++stride)
        {
          if (configs[scores[stride].config_index].ips < worst_ips)
          {
            worst_ips = configs[scores[stride].config_index].ips;
            worst_stride = stride;
          }
        }
        configs[scores[worst_stride].config_index].lease *= 2;
        configs[scores[worst_stride].config_index].state = Inactive;
        scores[worst_stride].reset_stats();
        scores[worst_stride].config_index = config_index;
        configs[scores[worst_stride].config_index].state = Live;
        gen_quadratic_interactions(worst_stride);
        current_champ = worst_stride;
        // Rare case breaks index_queue, best to just clear it
        while (!index_queue.empty()) { index_queue.pop(); };
      }
    }

    // If challenger is worse ('worse function from Chacha')
    if (worse(configs[config_index], champ_config)) { configs[config_index].state = Removed; }
  }
  if (champ_change) { exclusion_configs_oracle(); }
}

void interaction_config_manager::save_load(io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream msg;

  if (!read) msg << "_aml_cm_state " << current_state << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&current_state), sizeof(current_state), read, msg, text);

  if (!read) msg << "_aml_cm_county " << county << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&county), sizeof(county), read, msg, text);

  if (!read) msg << "_aml_cm_champ " << current_champ << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&current_champ), sizeof(current_champ), read, msg, text);

  size_t config_size;
  size_t ns_counter_size;
  size_t index_queue_size;
  size_t score_size;
  if (read)
  {
    // Load scores
    scores.clear();
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&score_size), sizeof(score_size), read, msg, text);
    for (size_t i = 0; i < score_size; ++i)
    {
      scored_config sc;
      sc.save_load_scored_config(model_file, read, text);
      scores.push_back(sc);
    }

    // Load configs
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&config_size), sizeof(config_size), read, msg, text);
    for (size_t i = 0; i < config_size; ++i)
    {
      size_t index;
      exclusion_config conf;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&index), sizeof(index), read, msg, text);
      conf.save_load_exclusion_config(model_file, read, text);
      configs[index] = conf;
    }

    // Load ns_counter
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&ns_counter_size), sizeof(ns_counter_size), read, msg, text);
    for (size_t i = 0; i < ns_counter_size; ++i)
    {
      namespace_index ns;
      size_t count;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&ns), sizeof(ns), read, msg, text);
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&count), sizeof(count), read, msg, text);
      ns_counter[ns] = count;
    }

    // Load index_queue
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&index_queue_size), sizeof(index_queue_size), read, msg, text);
    for (size_t i = 0; i < index_queue_size; ++i)
    {
      float priority;
      size_t index;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&priority), sizeof(priority), read, msg, text);
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&index), sizeof(index), read, msg, text);
      index_queue.push(std::make_pair(priority, index));
    }

    for (size_t stride = 0; stride < scores.size(); ++stride) { gen_quadratic_interactions(stride); }
  }
  else
  {
    // Save scores
    score_size = scores.size();
    msg << "score_size " << score_size << "\n";
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&score_size), sizeof(score_size), read, msg, text);
    for (auto& score : scores) { score.save_load_scored_config(model_file, read, text); }

    // Save configs
    config_size = configs.size();
    msg << "config_size " << config_size << "\n";
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&config_size), sizeof(config_size), read, msg, text);
    for (auto& ind_config : configs)
    {
      bin_text_read_write_fixed(model_file, (char*)&ind_config.first, sizeof(ind_config.first), read, msg, text);
      ind_config.second.save_load_exclusion_config(model_file, read, text);
    }

    // Save ns_counter
    ns_counter_size = ns_counter.size();
    msg << "ns_counter_size " << ns_counter_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&ns_counter_size), sizeof(ns_counter_size), read, msg, text);
    for (auto& ns_count : ns_counter)
    {
      bin_text_read_write_fixed(model_file, (char*)&ns_count.first, sizeof(ns_count.first), read, msg, text);
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&ns_count.second), sizeof(ns_count.second), read, msg, text);
    }

    // Save index_queue
    index_queue_size = index_queue.size();
    msg << "index_queue_size " << index_queue_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&index_queue_size), sizeof(index_queue_size), read, msg, text);
    while (!index_queue.empty())
    {
      auto& pri_ind = index_queue.top();
      bin_text_read_write_fixed(model_file, (char*)&pri_ind.first, sizeof(pri_ind.first), read, msg, text);
      bin_text_read_write_fixed(model_file, (char*)&pri_ind.second, sizeof(pri_ind.second), read, msg, text);
      index_queue.pop();
    }
  }
}

void interaction_config_manager::persist(metric_sink& metrics)
{
  metrics.int_metrics_list.emplace_back("test_county", county);
  metrics.int_metrics_list.emplace_back("current_champ", current_champ);
  for (size_t stride = 0; stride < scores.size(); ++stride)
  { scores[stride].persist(metrics, "_" + std::to_string(stride)); }
}

// This sets up example with correct ineractions vector
void interaction_config_manager::apply_config(example* ec, size_t stride)
{
  if (ec == nullptr) { return; }
  if (stride < max_live_configs) { ec->interactions = &(scores[stride].live_interactions); }
  else
  {
    THROW("fatal (automl): trying to apply a config higher than max configs allowed");
  }
}

void interaction_config_manager::revert_config(example* ec) { ec->interactions = nullptr; }

template <typename CMType, bool is_explore>
void predict_automl(automl<CMType>& data, multi_learner& base, multi_ex& ec)
{
  size_t champ_stride = data.cm->current_champ;
  for (example* ex : ec) { data.cm->apply_config(ex, champ_stride); }

  auto restore_guard = VW::scope_exit([&data, &ec, &champ_stride] {
    for (example* ex : ec) { data.cm->revert_config(ex); }
  });

  base.predict(ec, champ_stride);
}

// inner loop of learn driven by # MAX_CONFIGS
template <typename CMType>
void offset_learn(automl<CMType>& data, multi_learner& base, multi_ex& ec, const size_t stride, CB::cb_class& logged,
    size_t labelled_action)
{
  for (example* ex : ec) { data.cm->apply_config(ex, stride); }

  auto restore_guard = VW::scope_exit([&data, &ec, &stride] {
    for (example* ex : ec) { data.cm->revert_config(ex); }
  });

  if (!base.learn_returns_prediction) { base.predict(ec, stride); }

  base.learn(ec, stride);

  const auto& action_scores = ec[0]->pred.a_s;
  // cb_adf => first action is a greedy action
  const auto maxit = action_scores.begin();
  const uint32_t chosen_action = maxit->action;

  // extra asserts
  assert(chosen_action < ec.size());
  assert(labelled_action < ec.size());

  const float w = logged.probability > 0 ? 1 / logged.probability : 0;
  const float r = -logged.cost;

  data.cm->scores[stride].update(chosen_action == labelled_action ? w : 0, r);

  // cache the champ
  if (data.cm->current_champ == stride) { data.champ_a_s = std::move(ec[0]->pred.a_s); }
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(automl<CMType>& data, multi_learner& base, multi_ex& ec)
{
  assert(data.all->weights.sparse == false);

  bool is_learn = true;

  if (is_learn) { data.cm->county++; }
  // extra assert just bc
  assert(data.all->interactions.empty() == true);

  CB::cb_class logged{};
  size_t labelled_action = 0;
  if (is_learn)
  {
    const auto it = std::find_if(ec.begin(), ec.end(), [](example* item) { return !item->l.cb.costs.empty(); });

    if (it != ec.end())
    {
      logged = (*it)->l.cb.costs[0];
      labelled_action = std::distance(ec.begin(), it);
    }
  }

  config_manager_state current_state = data.cm->current_state;
  data.cm->one_step(ec);

  if (current_state == config_manager_state::Experimenting)
  {
    for (size_t stride = 0; stride < data.cm->scores.size(); ++stride)
    { offset_learn(data, base, ec, stride, logged, labelled_action); }
    // replace bc champ always gets cached
    ec[0]->pred.a_s = std::move(data.champ_a_s);
  }
  else
  {
    size_t champ = data.cm->current_champ;
    offset_learn(data, base, ec, champ, logged, labelled_action);
    // replace bc champ always gets cached
    ec[0]->pred.a_s = std::move(data.champ_a_s);
  }

  // extra: assert again just like at the top
  assert(data.all->interactions.empty() == true);
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
    size_t champ_stride = data.cm->current_champ;
    for (example* ex : ec) { data.cm->apply_config(ex, champ_stride); }

    auto restore_guard = VW::scope_exit([&data, &ec, &champ_stride] {
      for (example* ex : ec) { data.cm->revert_config(ex); }
    });

    data.adf_learner->print_example(all, ec);
  }

  VW::finish_example(all, ec);
}

template <typename CMType>
void save_load_aml(automl<CMType>& d, io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  d.cm->save_load(model_file, read, text);
}

// Basic implementation of scheduler to pick new configs when one runs out of lease.
// Highest priority will be picked first because of max-PQ implementation, this will
// be the config with the least exclusion. Note that all configs will run to lease
// before priorities and lease are reset.
float calc_priority_least_exclusion(size_t config_index, const std::map<size_t, exclusion_config>& configs,
    const std::map<namespace_index, size_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : configs.at(config_index).exclusions) { priority -= ns_counter.at(ns_pair.first); }
  return priority;
}

// Same as above, returns 0 (includes rest to remove unused variable warning)
float calc_priority_empty(size_t config_index, const std::map<size_t, exclusion_config>& configs,
    const std::map<namespace_index, size_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : configs.at(config_index).exclusions) { priority -= ns_counter.at(ns_pair.first); }
  return 0.f;
}

VW::LEARNER::base_learner* automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();

  size_t lease;
  size_t max_live_configs;
  std::string cm_type;
  std::string priority_type;

  option_group_definition new_options("Debug: automl reduction");
  new_options
      .add(make_option("automl", max_live_configs)
               .necessary()
               .keep()
               .default_value(3)
               .help("set number of live configs"))
      .add(make_option("lease", lease).keep().default_value(10).help("set initial lease for automl interactions"))
      .add(make_option("cm_type", cm_type).keep().default_value("interaction").help("set type of config manager"))
      .add(make_option("priority_type", priority_type)
               .keep()
               .default_value("none")
               .help("set type of config manager in {none, le}"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  float (*calc_priority)(size_t, const std::map<size_t, exclusion_config>&, const std::map<namespace_index, size_t>&);

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "le")
  {
    calc_priority = &calc_priority_least_exclusion;
  }
  else
  {
    THROW("Error: Invalid priority function provided");
  }

  auto cm = VW::make_unique<interaction_config_manager>(lease, max_live_configs, all.random_seed, calc_priority);
  auto data = VW::make_unique<automl<interaction_config_manager>>(std::move(cm));
  // all is not needed but good to have for testing purposes
  data->all = &all;
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

  // ask jack about flushing the cache, after mutating reductions
  // that might change

  details::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  // only this has been tested
  if (base_learner->is_multiline)
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
                  .set_prediction_type(base_learner->pred_type)
                  .set_label_type(label_type_t::cb)
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
}  // namespace VW
