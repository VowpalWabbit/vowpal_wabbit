// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "constant.h"  // NUM_NAMESPACES
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include <cfloat>

#include "distributionally_robust.h"
#include "io/logger.h"
#include "vw.h"

using namespace VW::config;
using namespace VW::LEARNER;
using interaction_vec = std::vector<std::vector<namespace_index>>;

namespace logger = VW::io::logger;

namespace VW
{
namespace test_red
{
const size_t MAX_CONFIGS = 3;
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

struct scored_config
{
  scored_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}

  void update(float w, float r)
  {
    update_count++;
    chisq.update(w, r);
    ips += r * w;
    last_w = w;
    last_r = r;
  }

  void save_load_aml(io_buf& model_file, bool read, bool text)
  {
    if (model_file.num_files() == 0) { return; }
    if (!read || true)
    {
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

      chisq.save_load(model_file, read, text);
    }
  }

  void persist(metric_sink& metrics, const std::string& suffix)
  {
    metrics.int_metrics_list.emplace_back("upcnt" + suffix, update_count);
    metrics.float_metrics_list.emplace_back("ips" + suffix, current_ips());
    distributionally_robust::ScoredDual sd = chisq.recompute_duals();
    metrics.float_metrics_list.emplace_back("bound" + suffix, static_cast<float>(sd.first));
    metrics.float_metrics_list.emplace_back("w" + suffix, last_w);
    metrics.float_metrics_list.emplace_back("r" + suffix, last_r);
  }

  float current_ips() { return ips / update_count; }

  void reset_stats()
  {
    chisq.reset(0.05, 0.999);
    ips = 0.0;
    last_w = 0.0;
    last_r = 0.0;
    update_count = 0;
  }

  VW::distributionally_robust::ChiSquared chisq;
  float ips = 0.0;
  float last_w = 0.0;
  float last_r = 0.0;
  size_t update_count = 0;
  int config_index = -1;
};

struct exclusion_config
{
  exclusion_config(size_t starting_budget = 10) : starting_budget(starting_budget), budget(starting_budget) {}
  std::set<namespace_index> exclusions;
  size_t starting_budget;
  size_t budget;
};

// all possible states of config_manager
enum config_state
{
  Idle,
  Collecting,
  Experimenting
};

struct oracle
{
  virtual interaction_vec gen_interactions(
      const std::map<namespace_index, size_t>&, const std::set<namespace_index>&) = 0;
};

struct quadratic_exclusion_oracle : oracle
{
  // This code is primarily borrowed from expand_quadratics_wildcard_interactions in
  // interactions.cc. It will generate interactions with -q :: and exclude namespaces
  // from the corresponding stride. This function can be swapped out depending on
  // preference of how to generate interactions from a given set of exclusions.
  interaction_vec gen_interactions(
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
};

// config_manager is a state machine (config_state) 'time' moves forward after a call into one_step()
// this can also be interpreted as a pre-learn() hook since it gets called by a learn() right before calling
// into its own base_learner.learn(). see learn_automl(...)
struct config_manager
{
  config_state current_state = Idle;
  size_t county = 0;
  size_t current_champ = 0;
  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, size_t> ns_counter;
  // Stores all configs in consideration (Map allows easy deletion unlike vector)
  std::map<size_t, exclusion_config> configs;
  scored_config scores[MAX_CONFIGS];
  interaction_vec live_interactions[MAX_CONFIGS];  // Live pre-allocated vectors in use
  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, int>> index_queue;
  const size_t max_live_configs = MAX_CONFIGS;
  quadratic_exclusion_oracle oc;
  size_t starting_budget;
  config_manager(size_t starting_budget)
  {
    this->starting_budget = starting_budget;
    exclusion_config conf(starting_budget);
    configs[0] = conf;
    scores[0].config_index = -2;
  }

  /*
    max_live_configs = 5;
    config_manager -has-> 10 configs it wants to test

    1 champ, 4 tests (give a budget 500 examples)
    ~ time passes ~ 500 later
    10 configs (5 are live, 5 are dormant)
     1) give more budget to live ones, in case they are almost good
     2) swap configs, what does it imply, the 4 are bad, add 4 new configs from the dormants
  */

  void one_step(multi_ex& ec)
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

  /*
    conf = initConfig('F','B')
    conf.append('C', 'G')

    conf.merge(conf2); -> normie merge/add ?
    conf.multiply('T') -> FT, FB, BT

    namespace count total -> scheduled experiments, meta-metrics of the previous experiments?
    example number for the last time it got used,

    atomized in a very nice way:
    prepare: for us to do beam search over the configuration space over diff heuristics
    -> pluggable, encapsulated
    -> expand config, create new model from config, find neighbours of this config

    // schema discovery
    // rotate configs with budget

    enumerate cnofigs, swap them in and out on a schedule, autoschema disvoery, but playing all

    reset and copy -- learner stack
    warmstart, reset to zero (or random?)
  */

  // Basic implementation of scheduler to pick new configs when one runs out of budget.
  // Highest priority will be picked first because of max-PQ implementation, this will
  // be the config with the least exclusion. Note that all configs will run to budget
  // before priorities and budget are reset.
  float get_priority(size_t config_index)
  {
    float priority = 0.f;
    for (auto ns : configs[config_index].exclusions) { priority -= ns_counter[ns]; }
    return priority;
  }

  // This will generate configs with all combinations of current namespaces. These
  // combinations will be held stored as 'exclusions' but can be used differently
  // depending on oracle design.
  void gen_configs(const multi_ex& ecs)
  {
    std::set<namespace_index> new_namespaces;
    for (example* ex : ecs)
    {
      for (auto& ns : ex->indices)
      {
        ns_counter[ns]++;
        if (ns_counter[ns] == 1) { new_namespaces.insert(ns); }
      }
    }
    for (auto& ns : new_namespaces)
    {
      std::set<std::set<namespace_index>> new_exclusions;
      for (auto& config : configs)
      {
        std::set<namespace_index> new_exclusion(config.second.exclusions);
        new_exclusion.insert(ns);
        new_exclusions.insert(new_exclusion);
      }
      for (auto& new_exclusion : new_exclusions)
      {
        int config_index = configs.size();
        exclusion_config conf(starting_budget);
        conf.exclusions = new_exclusion;
        configs[config_index] = conf;
        float priority = get_priority(config_index);
        index_queue.push(std::make_pair(priority, config_index));
      }
    }
  }

  // This function is triggered when all sets of interactions generated by the oracle have been tried and
  // reached their budget. It will then add inactive configs (stored in the config manager) to the queue
  // 'index_queue' which can be used to swap out live configs as they run out of budget. This functionality
  // may be better within the oracle, which could generate better priorities for different configs based
  // on ns_counter (which is updated as each example is processed)
  bool repopulate_index_queue()
  {
    std::set<int> live_indices;
    for (size_t stride = 0; stride < max_live_configs; ++stride) { live_indices.insert(scores[stride].config_index); }
    for (auto ind_config : configs)
    {
      int redo_index = ind_config.first;
      if (live_indices.find(redo_index) == live_indices.end())
      {
        float priority = get_priority(redo_index);
        index_queue.push(std::make_pair(priority, redo_index));
      }
    }
    return !index_queue.empty();
  }

  // This function defines the logic update the live configs' budgets, and to swap out
  // new configs when the budget runs out.
  void update_live_configs()
  {
    for (size_t stride = 0; stride < max_live_configs; ++stride)
    {
      int config_index = scores[stride].config_index;
      // Special case to initialize first champion to -q ::
      if (config_index == -2)
      {
        scores[stride].config_index = 0;
        live_interactions[stride] = oc.gen_interactions(ns_counter, configs[0].exclusions);
      }
      if (stride == current_champ) { continue; }
      if (config_index == -1 || configs[config_index].budget == 0)
      {
        if (config_index >= 0)
        {
          scores[stride].reset_stats();
          configs[config_index].starting_budget *= 2;
          configs[config_index].budget = configs[config_index].starting_budget;
        }
        // TODO: Add logic to erase index from configs map if wanted
        if (index_queue.empty() && !repopulate_index_queue()) { continue; }
        config_index = index_queue.top().second;
        index_queue.pop();
        scores[stride].config_index = config_index;
        // Regenerate interactions each time an exclusion is swapped in
        live_interactions[stride].clear();
        live_interactions[stride] = oc.gen_interactions(ns_counter, configs[config_index].exclusions);
        // We may also want to 0 out weights here? Currently keep all same in stride position
      }
      configs[config_index].budget--;
    }
  }

  void update_champ()
  {
    float temp_champ_ips = scores[current_champ].current_ips();

    // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
    for (size_t stride = 0; stride < max_live_configs; ++stride)
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

  void save_load_aml(io_buf& model_file, bool read, bool text)
  {
    if (model_file.num_files() == 0) { return; }
    if (!read || true)
    {
      // make sure we can deserialize based on dynamic values like max_live_configs
      // i.e. read/write first the quantity and then do the for loop
      for (size_t stride = 0; stride < max_live_configs; ++stride)
      { scores[stride].save_load_aml(model_file, read, text); }

      std::stringstream msg;
      if (!read) msg << "_aml_cm_state " << current_state << "\n";
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&current_state), sizeof(current_state), "", read, msg, text);

      if (!read) msg << "_aml_cm_county " << county << "\n";
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&county), sizeof(county), "", read, msg, text);

      if (!read) msg << "_aml_cm_champ " << current_champ << "\n";
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&current_champ), sizeof(current_champ), "", read, msg, text);
    }
  }

  void persist(metric_sink& metrics)
  {
    metrics.int_metrics_list.emplace_back("test_county", county);
    metrics.int_metrics_list.emplace_back("current_champ", current_champ);
    for (size_t stride = 0; stride < max_live_configs; ++stride)
    { scores[stride].persist(metrics, "_" + std::to_string(stride)); }
  }

  // This sets up example with correct ineractions vector
  void configure_interactions(example* ec, size_t stride)
  {
    if (ec == nullptr) return;
    ec->interactions = &(live_interactions[stride]);
  }

  void restore_interactions(example* ec) { ec->interactions = nullptr; }
};

struct tr_data
{
  config_manager cm;
  // all is not needed but good to have for testing purposes
  vw* all;
  // problem multiplier
  size_t pm = MAX_CONFIGS;
  // to simulate printing in cb_explore_adf
  multi_learner* adf_learner;
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
  tr_data(size_t starting_budget) : cm(starting_budget) {}
};

template <bool is_explore>
void predict_automl(tr_data& data, multi_learner& base, multi_ex& ec)
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
    tr_data& data, multi_learner& base, multi_ex& ec, const size_t stride, CB::cb_class& logged, size_t labelled_action)
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
void learn_automl(tr_data& data, multi_learner& base, multi_ex& ec)
{
  assert(data.all->weights.sparse == false);

  bool is_learn = true;
  // assert we learn twice
  assert(data.pm == data.cm.max_live_configs);

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
    for (size_t stride = 0; stride < data.cm.max_live_configs; ++stride)
    {
      if (data.cm.scores[stride].config_index < 0) { continue; }
      offset_learn(data, base, ec, stride, logged, labelled_action);
    }
    // replace with prediction depending on current_champ
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

void persist(tr_data& data, metric_sink& metrics) { data.cm.persist(metrics); }

void finish_example(vw& all, tr_data& data, multi_ex& ec)
{
  data.adf_learner->print_example(all, ec);
  VW::finish_example(all, ec);
}

void save_load_aml(tr_data& d, io_buf& model_file, bool read, bool text)
{
  if (model_file.num_files() == 0) { return; }
  if (!read || true) { d.cm.save_load_aml(model_file, read, text); }
}

VW::LEARNER::base_learner* test_red_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  vw& all = *stack_builder.get_all_pointer();

  size_t test_red;
  size_t starting_budget;

  option_group_definition new_options("Debug: test reduction");
  new_options.add(make_option("test_red", test_red).necessary().keep().help("set default champion (0 or 1)"))
      .add(make_option("budget", starting_budget)
               .keep()
               .default_value(10)
               .help("set initial budget for automl interactions"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  auto data = VW::make_unique<tr_data>(starting_budget);
  // all is not needed but good to have for testing purposes
  data->all = &all;
  data->cm.current_champ = test_red;

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
    auto ppw = data->pm;

    auto* l =
        make_reduction_learner(std::move(data), as_multiline(base_learner), learn_automl<true>, predict_automl<true>,
            stack_builder.get_setupfn_name(test_red_setup))
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

}  // namespace test_red
}  // namespace VW
