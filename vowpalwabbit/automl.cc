// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "automl.h"
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include "io/logger.h"
#include "vw.h"
#include "vw_versions.h"

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;

namespace logger = VW::io::logger;

namespace VW
{
namespace automl
{
namespace helper
{
// add an interaction to an existing instance
/*void add_interaction(interaction_vec& interactions, namespace_index first, namespace_index second)
{
  std::vector<namespace_index> vect;
  vect.push_back(first);
  vect.push_back(second);
  interactions.push_back(vect);
}*/

// fail if incompatible reductions got setup
// inefficient, address later
// references global all interactions
void fail_if_enabled(vw& all, std::string name)
{
  std::vector<std::string> enabled_reductions;
  if (all.l != nullptr) all.l->get_enabled_reductions(enabled_reductions);

  if (std::find(enabled_reductions.begin(), enabled_reductions.end(), name) != enabled_reductions.end())
    THROW("plz no bad stack" + name);
}

bool cmpf(float A, float B, float epsilon = 0.001f) { return (fabs(A - B) < epsilon); }

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
}  // namespace helper

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
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&ips), sizeof(ips), "", read, msg, text);

  if (!read) msg << "_aml_config_count " << update_count << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&update_count), sizeof(update_count), "", read, msg, text);

  if (!read) msg << "_aml_config_lastw " << last_w << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&last_w), sizeof(last_w), "", read, msg, text);

  if (!read) msg << "_aml_config_lastr " << last_r << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&last_r), sizeof(last_r), "", read, msg, text);

  if (!read) msg << "_aml_config_index " << config_index << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&config_index), sizeof(config_index), "", read, msg, text);

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

float scored_config::current_ips() { return ips / update_count; }

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
        model_file, reinterpret_cast<char*>(&exclusion_size), sizeof(exclusion_size), "", read, msg, text);
    for (size_t i = 0; i < exclusion_size; ++i)
    {
      namespace_index ns;
      bin_text_read_write_fixed(model_file, (char*)&ns, sizeof(ns), "", read, msg, text);
      exclusions.insert(ns);
    }
  }
  else
  {
    exclusion_size = exclusions.size();
    msg << "exclusion_size " << exclusion_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&exclusion_size), sizeof(exclusion_size), "", read, msg, text);
    for (const auto& ns : exclusions)
    { bin_text_read_write_fixed(model_file, (char*)&ns, sizeof(ns), "", read, msg, text); }
  }

  if (!read || true)
  {
    if (!read) msg << "_aml_budget " << budget << "\n";
    bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&budget), sizeof(budget), "", read, msg, text);
  }
}

// This code is primarily borrowed from expand_quadratics_wildcard_interactions in
// interactions.cc. It will generate interactions with -q :: and exclude namespaces
// from the corresponding stride. This function can be swapped out depending on
// preference of how to generate interactions from a given set of exclusions.
// Transforms exclusions -> interactions expected by VW.
interaction_vec quadratic_exclusion_oracle::gen_interactions(
    const std::map<namespace_index, size_t>& ns_counter, const std::set<namespace_index>& exclusions)
{
  std::set<std::vector<namespace_index>> interactions;

  for (auto it = ns_counter.begin(); it != ns_counter.end(); ++it)
  {
    auto idx1 = (*it).first;
    if (exclusions.find(idx1) != exclusions.end()) { continue; }
    interactions.insert({idx1, idx1});

    for (auto jt = it; jt != ns_counter.end(); ++jt)
    {
      auto idx2 = (*jt).first;
      if (exclusions.find(idx2) != exclusions.end()) { continue; }
      interactions.insert({idx1, idx2});
      interactions.insert({idx2, idx2});
    }
  }
  interaction_vec v = interaction_vec(interactions.begin(), interactions.end());
  return v;
}

// config_manager is a state machine (config_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own base_learner.learn(). see learn_automl(...)
config_manager::config_manager(size_t budget, const size_t live_configs) : budget(budget), live_configs(live_configs)
{
  exclusion_config conf(budget);
  configs[0] = conf;
  scored_config sc;
  sc.config_index = 0;
  scores.push_back(sc);
  interaction_vec v;
  live_interactions.push_back(v);
}

void config_manager::one_step(multi_ex& ec)
{
  switch (current_state)
  {
    case Idle:
      current_state = Experimenting;
      break;

    case Collecting:
      break;

    case Experimenting:
      gen_configs(ec);
      update_live_configs();
      update_champ();
      break;

    default:
      break;
  }
}

// Basic implementation of scheduler to pick new configs when one runs out of budget.
// Highest priority will be picked first because of max-PQ implementation, this will
// be the config with the least exclusion. Note that all configs will run to budget
// before priorities and budget are reset.
float config_manager::get_priority(size_t config_index)
{
  float priority = 0.f;
  for (const auto& ns : configs[config_index].exclusions) { priority -= ns_counter[ns]; }
  return priority;
}

// This will generate configs with all combinations of current namespaces. These
// combinations will be held stored as 'exclusions' but can be used differently
// depending on oracle design.
void config_manager::gen_configs(const multi_ex& ecs)
{
  std::set<namespace_index> new_namespaces;
  // Count all namepsace seen in current example
  for (const example* ex : ecs)
  {
    for (const auto& ns : ex->indices)
    {
      ns_counter[ns]++;
      if (ns_counter[ns] == 1) { new_namespaces.insert(ns); }
    }
  }
  // Add new configs if new namespace are seen
  for (const auto& ns : new_namespaces)
  {
    std::set<std::set<namespace_index>> new_exclusions;
    for (const auto& config : configs)
    {
      std::set<namespace_index> new_exclusion(config.second.exclusions);
      new_exclusion.insert(ns);
      new_exclusions.insert(new_exclusion);
    }
    for (const auto& new_exclusion : new_exclusions)
    {
      size_t config_index = configs.size();
      exclusion_config conf(budget);
      conf.exclusions = new_exclusion;
      configs[config_index] = conf;
      float priority = get_priority(config_index);
      index_queue.push(std::make_pair(priority, config_index));
    }
  }
  // Regenerate interactions if new namespaces are seen
  if (!new_namespaces.empty())
  {
    for (size_t stride = 0; stride < scores.size(); ++stride)
    {
      live_interactions[stride].clear();
      live_interactions[stride] = oc.gen_interactions(ns_counter, configs[scores[stride].config_index].exclusions);
    }
  }
}

// This function is triggered when all sets of interactions generated by the oracle have been tried and
// reached their budget. It will then add inactive configs (stored in the config manager) to the queue
// 'index_queue' which can be used to swap out live configs as they run out of budget. This functionality
// may be better within the oracle, which could generate better priorities for different configs based
// on ns_counter (which is updated as each example is processed)
bool config_manager::repopulate_index_queue()
{
  std::set<size_t> live_indices;
  for (const auto& score : scores) { live_indices.insert(score.config_index); }
  for (const auto& ind_config : configs)
  {
    size_t redo_index = ind_config.first;
    if (live_indices.find(redo_index) == live_indices.end())
    {
      float priority = get_priority(redo_index);
      index_queue.push(std::make_pair(priority, redo_index));
    }
  }
  return !index_queue.empty();
}

void config_manager::handle_empty_budget(size_t stride)
{
  size_t config_index = scores[stride].config_index;
  scores[stride].reset_stats();
  configs[config_index].budget *= 2;
  // TODO: Add logic to erase index from configs map if wanted
  if (index_queue.empty() && !repopulate_index_queue()) { return; }
  config_index = index_queue.top().second;
  index_queue.pop();
  scores[stride].config_index = config_index;
  // Regenerate interactions each time an exclusion is swapped in
  live_interactions[stride].clear();
  live_interactions[stride] = oc.gen_interactions(ns_counter, configs[config_index].exclusions);
  // We may also want to 0 out weights here? Currently keep all same in stride position
}

// This function defines the logic update the live configs' budgets, and to swap out
// new configs when the budget runs out.
void config_manager::update_live_configs()
{
  for (size_t stride = 0; stride < live_configs; ++stride)
  {
    size_t config_index = scores[stride].config_index;
    if (stride == current_champ) { continue; }
    if (scores.size() <= stride)
    {
      if (index_queue.empty() && !repopulate_index_queue()) { return; }
      scored_config sc;
      sc.config_index = index_queue.top().second;
      index_queue.pop();
      scores.push_back(sc);
      interaction_vec v = oc.gen_interactions(ns_counter, configs[sc.config_index].exclusions);
      live_interactions.push_back(v);
    }
    else if (scores[stride].update_count >= configs[config_index].budget)
    {
      handle_empty_budget(stride);
    }
  }
}

void config_manager::update_champ()
{
  float temp_champ_ips = scores[current_champ].current_ips();

  // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
  for (size_t stride = 0; stride < scores.size(); ++stride)
  {
    if (stride == current_champ) continue;
    distributionally_robust::ScoredDual sd = scores[stride].chisq.recompute_duals();
    float challenger_l_bound = static_cast<float>(sd.first);
    if (temp_champ_ips < challenger_l_bound)
    {
      current_champ = stride;  // champ switch
      temp_champ_ips = challenger_l_bound;
    }
  }
}

void config_manager::save_load_config_manager(io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  std::stringstream msg;

  if (!read) msg << "_aml_cm_state " << current_state << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&current_state), sizeof(current_state), "", read, msg, text);

  if (!read) msg << "_aml_cm_county " << county << "\n";
  bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&county), sizeof(county), "", read, msg, text);

  if (!read) msg << "_aml_cm_champ " << current_champ << "\n";
  bin_text_read_write_fixed(
      model_file, reinterpret_cast<char*>(&current_champ), sizeof(current_champ), "", read, msg, text);

  size_t config_size;
  size_t ns_counter_size;
  size_t index_queue_size;
  size_t score_size;
  if (read)
  {
    // Load scores
    scores.clear();
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&score_size), sizeof(score_size), "", read, msg, text);
    for (size_t i = 0; i < score_size; ++i)
    {
      scored_config sc;
      sc.save_load_scored_config(model_file, read, text);
      scores.push_back(sc);
    }

    // Load configs
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&config_size), sizeof(config_size), "", read, msg, text);
    for (size_t i = 0; i < config_size; ++i)
    {
      size_t index;
      exclusion_config conf;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&index), sizeof(index), "", read, msg, text);
      conf.save_load_exclusion_config(model_file, read, text);
      configs[index] = conf;
    }

    // Load ns_counter
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&ns_counter_size), sizeof(ns_counter_size), "", read, msg, text);
    for (size_t i = 0; i < ns_counter_size; ++i)
    {
      namespace_index ns;
      size_t count;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&ns), sizeof(ns), "", read, msg, text);
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&count), sizeof(count), "", read, msg, text);
      ns_counter[ns] = count;
    }

    // Load index_queue
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&index_queue_size), sizeof(index_queue_size), "", read, msg, text);
    for (size_t i = 0; i < index_queue_size; ++i)
    {
      float priority;
      size_t index;
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&priority), sizeof(priority), "", read, msg, text);
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&index), sizeof(index), "", read, msg, text);
      index_queue.push(std::make_pair(priority, index));
    }

    // Regenerate interactions vectors
    live_interactions.clear();
    for (size_t stride = 0; stride < scores.size(); ++stride)
    {
      interaction_vec v = oc.gen_interactions(ns_counter, configs[scores[stride].config_index].exclusions);
      live_interactions.push_back(v);
    }
  }
  else
  {
    // Save scores
    score_size = scores.size();
    msg << "score_size " << score_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&score_size), sizeof(score_size), "", read, msg, text);
    for (auto& score : scores) { score.save_load_scored_config(model_file, read, text); }

    // Save configs
    config_size = configs.size();
    msg << "config_size " << config_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&config_size), sizeof(config_size), "", read, msg, text);
    for (auto& ind_config : configs)
    {
      bin_text_read_write_fixed(model_file, (char*)&ind_config.first, sizeof(ind_config.first), "", read, msg, text);
      ind_config.second.save_load_exclusion_config(model_file, read, text);
    }

    // Save ns_counter
    ns_counter_size = ns_counter.size();
    msg << "ns_counter_size " << ns_counter_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&ns_counter_size), sizeof(ns_counter_size), "", read, msg, text);
    for (auto& ns_count : ns_counter)
    {
      bin_text_read_write_fixed(model_file, (char*)&ns_count.first, sizeof(ns_count.first), "", read, msg, text);
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&ns_count.second), sizeof(ns_count.second), "", read, msg, text);
    }

    // Save index_queue
    index_queue_size = index_queue.size();
    msg << "index_queue_size " << index_queue_size << "\n";
    bin_text_read_write_fixed(
        model_file, reinterpret_cast<char*>(&index_queue_size), sizeof(index_queue_size), "", read, msg, text);
    while (!index_queue.empty())
    {
      auto& pri_ind = index_queue.top();
      bin_text_read_write_fixed(model_file, (char*)&pri_ind.first, sizeof(pri_ind.first), "", read, msg, text);
      bin_text_read_write_fixed(model_file, (char*)&pri_ind.second, sizeof(pri_ind.second), "", read, msg, text);
      index_queue.pop();
    }
  }
}

void config_manager::persist(metric_sink& metrics)
{
  metrics.int_metrics_list.emplace_back("test_county", county);
  metrics.int_metrics_list.emplace_back("current_champ", current_champ);
  for (size_t stride = 0; stride < scores.size(); ++stride)
  { scores[stride].persist(metrics, "_" + std::to_string(stride)); }
}

// This sets up example with correct ineractions vector
void config_manager::configure_interactions(example* ec, size_t stride)
{
  if (ec == nullptr) return;
  ec->interactions = &(live_interactions[stride]);
}

void config_manager::restore_interactions(example* ec) { ec->interactions = nullptr; }

template <bool is_explore>
void predict_automl(automl& data, multi_learner& base, multi_ex& ec)
{
  size_t champ_stride = data.cm.current_champ;
  for (example* ex : ec) { data.cm.configure_interactions(ex, champ_stride); }

  auto restore_guard = VW::scope_exit([&data, &ec, &champ_stride] {
    for (example* ex : ec) { data.cm.restore_interactions(ex); }
  });

  base.predict(ec, champ_stride);
}

// inner loop of learn driven by # MAX_CONFIGS
void offset_learn(
    automl& data, multi_learner& base, multi_ex& ec, const size_t stride, CB::cb_class& logged, size_t labelled_action)
{
  assert(ec[0]->interactions == nullptr);

  for (example* ex : ec) { data.cm.configure_interactions(ex, stride); }

  auto restore_guard = VW::scope_exit([&data, &ec, &stride] {
    for (example* ex : ec) { data.cm.restore_interactions(ex); }
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

  data.cm.scores[stride].update(chosen_action == labelled_action ? w : 0, r);

  // cache the champ
  if (data.cm.current_champ == stride) { data.champ_a_s = std::move(ec[0]->pred.a_s); }
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <bool is_explore>
void learn_automl(automl& data, multi_learner& base, multi_ex& ec)
{
  assert(data.all->weights.sparse == false);

  bool is_learn = true;

  if (is_learn) { data.cm.county++; }
  // extra assert just bc
  assert(data.all->interactions.empty() == true);

  // we force parser to set always as nullptr, see change in parser.cc
  assert(ec[0]->interactions == nullptr);
  // that way we can modify all.interactions without parser caring

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

  config_state current_state = data.cm.current_state;
  data.cm.one_step(ec);

  if (current_state == Experimenting)
  {
    for (size_t stride = 0; stride < data.cm.scores.size(); ++stride)
    { offset_learn(data, base, ec, stride, logged, labelled_action); }
    // replace bc champ always gets cached
    ec[0]->pred.a_s = std::move(data.champ_a_s);
  }
  else
  {
    size_t champ = data.cm.current_champ;
    offset_learn(data, base, ec, champ, logged, labelled_action);
    // replace bc champ always gets cached
    ec[0]->pred.a_s = std::move(data.champ_a_s);
  }

  // extra: assert again just like at the top
  assert(data.all->interactions.empty() == true);
  assert(ec[0]->interactions == nullptr);
}

void persist(automl& data, metric_sink& metrics) { data.cm.persist(metrics); }

void finish_example(vw& all, automl& data, multi_ex& ec)
{
  data.adf_learner->print_example(all, ec);
  VW::finish_example(all, ec);
}

void save_load_aml(automl& d, io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  if (!read || d.model_file_version >= VERSION_FILE_WITH_AUTOML)
  { d.cm.save_load_config_manager(model_file, read, text); }
}

VW::LEARNER::base_learner* automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();

  size_t budget;
  size_t live_configs;

  option_group_definition new_options("Debug: automl reduction");
  new_options
      .add(make_option("automl", live_configs).necessary().keep().default_value(3).help("set number of live configs"))
      .add(make_option("budget", budget).keep().default_value(10).help("set initial budget for automl interactions"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  auto data = VW::make_unique<automl>(budget, all.model_file_ver, live_configs);
  // all is not needed but good to have for testing purposes
  data->all = &all;
  assert(live_configs <= MAX_CONFIGS);

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

  helper::fail_if_enabled(all, "ccb_explore_adf");
  helper::fail_if_enabled(all, "audit_regressor");
  helper::fail_if_enabled(all, "baseline");
  helper::fail_if_enabled(all, "cb_explore_adf_rnd");
  helper::fail_if_enabled(all, "cb_to_cb_adf");
  helper::fail_if_enabled(all, "cbify");
  helper::fail_if_enabled(all, "replay_c");
  helper::fail_if_enabled(all, "replay_b");
  helper::fail_if_enabled(all, "replay_m");
  // fail_if_enabled(all, "gd");
  // fail_if_enabled(all, "generate_interactions");
  helper::fail_if_enabled(all, "memory_tree");
  helper::fail_if_enabled(all, "new_mf");
  helper::fail_if_enabled(all, "nn");
  helper::fail_if_enabled(all, "stage_poly");

  // only this has been tested
  if (base_learner->is_multiline)
  {
    // fetch cb_explore_adf to call directly into the print routine twice
    data->adf_learner = as_multiline(base_learner->get_learner_by_name_prefix("cb_explore_adf_"));
    auto ppw = MAX_CONFIGS;

    auto* l =
        make_reduction_learner(std::move(data), as_multiline(base_learner), learn_automl<true>, predict_automl<true>,
            stack_builder.get_setupfn_name(automl_setup))
            .set_params_per_weight(ppw)  // refactor pm
            .set_finish_example(finish_example)
            .set_save_load(save_load_aml)
            .set_persist_metrics(persist)
            .set_prediction_type(base_learner->pred_type)
            .set_label_type(label_type_t::cb)
            .set_learn_returns_prediction(true)
            .build();

    return make_base(*l);
  }
  else
  {
    // not implemented yet
    THROW("not supported");
  }
}

}  // namespace automl
}  // namespace VW
