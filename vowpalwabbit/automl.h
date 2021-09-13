#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reductions_fwd.h"
#include "distributionally_robust.h"
#include "constant.h"  // NUM_NAMESPACES
#include "metric_sink.h"
#include "action_score.h"
#include "version.h"
#include <map>
#include <set>
#include <queue>

namespace VW
{
namespace test_red
{
using namespace_index = unsigned char;
using interaction_vec = std::vector<std::vector<namespace_index>>;

VW::LEARNER::base_learner* test_red_setup(VW::setup_base_i& stack_builder);

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
void add_interaction(std::vector<std::vector<namespace_index>>&, namespace_index, namespace_index);
void fail_if_enabled(vw&, std::string);
bool cmpf(float, float, float);
// void print_weights_nonzero(vw*, size_t, dense_parameters&);
}  // namespace helper

const size_t MAX_CONFIGS = 3;

struct scored_config
{
  scored_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}

  void update(float w, float r);

  void save_load_scored_config(io_buf& model_file, bool read, bool text);

  void persist(metric_sink& metrics, const std::string& suffix);

  float current_ips();

  void reset_stats();

  VW::distributionally_robust::ChiSquared chisq;
  float ips = 0.0;
  float last_w = 0.0;
  float last_r = 0.0;
  size_t update_count = 0;
  int config_index = -1;
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
  interaction_vec gen_interactions(
      const std::map<namespace_index, size_t>& ns_counter, const std::set<namespace_index>& exclusions);
};

struct exclusion_config
{
  exclusion_config(size_t budget = 10) : budget(budget) {}
  std::set<namespace_index> exclusions;
  size_t budget;

  void save_load_exclusion_config(io_buf& model_file, bool read, bool text);
};


struct config_manager
{
  config_state current_state = Idle;
  size_t county = 0;
  size_t current_champ = 0;
  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, size_t> ns_counter;
  // Stores all configs in consideration (Map allows easy deletion unlike vector)
  std::map<int, exclusion_config> configs;
  scored_config scores[MAX_CONFIGS];
  interaction_vec live_interactions[MAX_CONFIGS];  // Live pre-allocated vectors in use
  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, int>> index_queue;
  const size_t max_live_configs = MAX_CONFIGS;
  quadratic_exclusion_oracle oc;
  size_t budget;

  config_manager(size_t starting_budget);

  void one_step(multi_ex& ec);

  float get_priority(int config_index);

  void gen_configs(const multi_ex& ecs);

  bool repopulate_index_queue();

  void update_live_configs();

  void update_champ();

  void save_load_config_manager(io_buf& model_file, bool read, bool text);

  void persist(metric_sink& metrics);

  void configure_interactions(example* ec, size_t stride);

  void restore_interactions(example* ec);

  void handle_empty_buget(size_t stride);
};

struct tr_data
{
  config_manager cm;
  // all is not needed but good to have for testing purposes
  vw* all;
  // problem multiplier
  size_t pm = MAX_CONFIGS;
  // to simulate printing in cb_explore_adf
  LEARNER::multi_learner* adf_learner;
  VW::version_struct model_file_version;
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
  tr_data(size_t starting_budget, VW::version_struct model_file_version) : cm(starting_budget), model_file_version(model_file_version) {}
};


}  // namespace test_red
}  // namespace VW
