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

namespace logger = VW::io::logger;

namespace VW
{
namespace test_red
{
namespace helper
{
// // for debugging purposes
// void print_interactions(example* ec)
// {
//   if (ec == nullptr) return;
//   if (ec->interactions == nullptr) return;

//   std::cerr << "p:";  // << ec->interactions;

//   for (std::vector<namespace_index> v : *(ec->interactions))
//   {
//     for (namespace_index c : v) { std::cerr << " interaction:" << c << ","; }
//   }
//   std::cerr << std::endl;
// }

// // useful to understand what namespaces are used in the examples we are given
// // this can evolve to feed in data to generate possible interactions
// void print_all_namespaces_in_examples(multi_ex& exs)
// {
//   for (example* ex : exs)
//   {
//     for (auto i : ex->indices) { std::cerr << i << ", "; }
//     std::cerr << std::endl;
//   }
// }

// void print_all_preds(example& ex, size_t i)
// {
//   const auto& preds = ex.pred.a_s;
//   std::cerr << "config_" << i << ": ";
//   for (uint32_t i = 0; i < preds.size(); i++)
//   {
//     std::cerr << preds[i].action << "(" << preds[i].score << ")"
//               << ", ";
//   }
//   std::cerr << std::endl;
// }

// add an interaction to an existing instance
void add_interaction(
    std::vector<std::vector<namespace_index>>& interactions, namespace_index first, namespace_index second)
{
  std::vector<namespace_index> vect;
  vect.push_back(first);
  vect.push_back(second);
  interactions.push_back(vect);
}

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
}  // namespace helper

// struct config_desc
// {
//   std::vector<std::vector<namespace_index>> interactions_1;
//   size_t budget = 0;
//   size_t last_time_ran = 0; // in # of examples
// }

struct single_config
{
  single_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}

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

      chisq.save_load_aml(model_file, read, text);
    }
  }

  void persist(metric_sink& metrics, const std::string& suffix)
  {
    metrics.int_metrics_list.emplace_back("upcnt" + suffix, update_count);
    metrics.float_metrics_list.emplace_back("ips" + suffix, current_ips());
    distributionally_robust::ScoredDual sd = chisq.recompute_duals();
    metrics.float_metrics_list.emplace_back("bound" + suffix, (float)(sd.first));
    metrics.float_metrics_list.emplace_back("w" + suffix, last_w);
    metrics.float_metrics_list.emplace_back("r" + suffix, last_r);
  }

  float current_ips() { return ips / update_count; }

  float ips = 0.0;
  VW::distributionally_robust::ChiSquared chisq;
  float last_w = 0.0;
  float last_r = 0.0;
  size_t update_count = 0;
  // add budget
};

enum config_state
{
  Idle,
  Collecting,
  Experimenting
};

struct config_manager
{
  config_state current_state = Idle;
  size_t county = 0;
  size_t current_champ = 0;
  single_config configs[2];

  size_t ns_counter[NUM_NAMESPACES] = {0};

  const size_t max_live_configs = 2;
  std::vector<std::vector<namespace_index>> interactions_1;
  std::vector<std::vector<namespace_index>> empty_interactions;

  void one_step()
  {
    switch (current_state)
    {
      case Idle:
        current_state = Experimenting;
        break;

      case Collecting:
        break;

      case Experimenting:
        evaluate();
        break;

      default:
        break;
    }
  }

  void count_ns(multi_ex& exs)
  {
    for (example* ex : exs)
    {
      for (auto i : ex->indices) { ns_counter[i]++; }
    }
  }

  /*
    namespace count total -> scheduled experiments, meta-metrics of the previous experiments?
    example number for the last time it got used,

    atomized in a very nice way:
    prepare: for us to do beam search over the configuration space over diff heuristics
    -> pluggable, encapsulated
    -> expand config, create new model from config, find neighbours of this config
  */

  bool add_highest_two_ns(std::vector<std::vector<namespace_index>>& interactions)
  {
    size_t max_1 = 0;
    size_t max_2 = 0;
    for (size_t i = 0; i < NUM_NAMESPACES; i++)
    {
      if (ns_counter[i] != 0 && i != 128)  // not sure why
      {
        if (max_1 == 0)
          max_1 = i;
        else if (max_2 == 0)
          max_2 = i;
        else if (ns_counter[max_1] < ns_counter[i])
          max_1 = i;
        else if (ns_counter[max_2] < ns_counter[i])
          max_2 = i;
      }
    }
    if (max_2 != 0)
    {
      helper::add_interaction(interactions, (namespace_index)max_1, (namespace_index)max_2);
      return true;
    }

    return false;
  }

  void evaluate()
  {
    if (interactions_1.empty())
    {
      bool result = add_highest_two_ns(interactions_1);
      if (result) assert(interactions_1.size() > 0);
    }

    float temp_champ_ips = configs[current_champ].current_ips();

    // compare lowerbound of any challenger to the ips of the champ, and switch whenever when the LB beats the champ
    for (size_t i = 0; i < max_live_configs; i++)
    {
      if (i == current_champ) continue;

      distributionally_robust::ScoredDual sd = configs[i].chisq.recompute_duals();
      float challenger_l_bound = (float)sd.first;
      if (temp_champ_ips < challenger_l_bound)
      {
        current_champ = i;  // champ switch
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
      for (size_t i = 0; i < max_live_configs; i++) { configs[i].save_load_aml(model_file, read, text); }

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

    const std::string suffixes[5] = {"_1", "_2", "_3", "_4", "_5"};

    for (size_t i = 0; i < max_live_configs; i++) { configs[i].persist(metrics, suffixes[i]); }
  }

  void configure_interactions(example* ec, size_t config_number)
  {
    if (ec == nullptr) return;

    if (config_number == 1)
    {
      ec->interactions = &(interactions_1);
      // std::cerr << config_number << "int:" << ec->interactions <<"s"<< ec->interactions->size() << std::endl;
    }
    else if (config_number == 0)
    {
      ec->interactions = &(empty_interactions);
      // std::cerr << config_number << "int:" << ec->interactions <<"s"<< ec->interactions->size() << std::endl;
    }
  }

  void restore_interactions(example* ec) { ec->interactions = nullptr; }
};

struct tr_data
{
  config_manager cm;
  // all is not needed but good to have for testing purposes
  vw* all;
  // problem multiplier
  size_t pm = 2;
  // to simulate printing in cb_explore_adf
  multi_learner* adf_learner;
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
};

template <bool is_explore>
void predict_automl(tr_data& data, multi_learner& base, multi_ex& ec)
{
  data.cm.count_ns(ec);
  size_t i = data.cm.current_champ;
  for (example* ex : ec) { data.cm.configure_interactions(ex, i); }

  // assert that the config is set correctly
  if (i == 0) { assert(ec[0]->interactions->empty() == true); }

  auto restore_guard = VW::scope_exit([&data, &ec, &i] {
    for (example* ex : ec) { data.cm.restore_interactions(ex); }
  });

  base.predict(ec, i);
}

void actual_learn(
    tr_data& data, multi_learner& base, multi_ex& ec, size_t i, CB::cb_class& logged, size_t labelled_action)
{
  assert(ec[0]->interactions == nullptr);

  for (example* ex : ec) { data.cm.configure_interactions(ex, i); }

  if (i == 0) { assert(ec[0]->interactions->empty() == true); }

  auto restore_guard = VW::scope_exit([&data, &ec, &i] {
    for (example* ex : ec) { data.cm.restore_interactions(ex); }
  });

  if (!base.learn_returns_prediction) { base.predict(ec, i); }

  base.learn(ec, i);

  const auto& action_scores = ec[0]->pred.a_s;
  // cb_adf => first action is a greedy action
  const auto maxit = action_scores.begin();
  const uint32_t chosen_action = maxit->action;

  // extra asserts
  assert(chosen_action < ec.size());
  assert(labelled_action < ec.size());

  const float w = logged.probability > 0 ? 1 / logged.probability : 0;
  const float r = -logged.cost;

  data.cm.configs[i].update(chosen_action == labelled_action ? w : 0, r);

  // cache the champ
  if (data.cm.current_champ == i) { data.champ_a_s = std::move(ec[0]->pred.a_s); }
}

template <bool is_explore>
void learn_automl(tr_data& data, multi_learner& base, multi_ex& ec)
{
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

  data.cm.one_step();

  if (data.cm.current_state == Experimenting)
  {
    for (size_t i = 0; i < data.pm; i++) { actual_learn(data, base, ec, i, logged, labelled_action); }
    // replace with prediction depending on current_champ
    ec[0]->pred.a_s = std::move(data.champ_a_s);
  }
  else
  {
    size_t champ = data.cm.current_champ;
    actual_learn(data, base, ec, champ, logged, labelled_action);
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
  auto data = VW::make_unique<tr_data>();

  option_group_definition new_options("Debug: test reduction");
  new_options.add(make_option("test_red", test_red).necessary().keep().help("set default champion (0 or 1)"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

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
