// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

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
namespace automl
{
using namespace_index = unsigned char;
using interaction_vec = std::vector<std::vector<namespace_index>>;

VW::LEARNER::base_learner* automl_setup(VW::setup_base_i&);

namespace helper
{
void fail_if_enabled(vw&, const std::string&);
bool cmpf(float, float, float);
// void print_weights_nonzero(vw*, size_t, dense_parameters&);
}  // namespace helper

constexpr size_t MAX_CONFIGS = 10;

struct scored_config
{
  VW::distributionally_robust::ChiSquared chisq;
  float ips = 0.0;
  float last_w = 0.0;
  float last_r = 0.0;
  size_t update_count = 0;
  size_t config_index = 0;
  interaction_vec live_interactions;  // Live pre-allocated vectors in use

  scored_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}

  void update(float w, float r);
  void save_load_scored_config(io_buf&, bool, bool);
  void persist(metric_sink&, const std::string&);
  float current_ips() const;
  void reset_stats();
};

struct oracle
{
  virtual interaction_vec gen_interactions(
      const std::map<namespace_index, size_t>&, const std::map<namespace_index, std::set<namespace_index>>&) const = 0;
};

struct quadratic_exclusion_oracle : oracle
{
  interaction_vec gen_interactions(
      const std::map<namespace_index, size_t>&, const std::map<namespace_index, std::set<namespace_index>>&) const;
};

struct exclusion_config
{
  std::map<namespace_index, std::set<namespace_index>> exclusions;
  size_t budget;

  exclusion_config(size_t budget = 10) : budget(budget) {}

  void save_load_exclusion_config(io_buf&, bool, bool);
};

// all possible states of config_manager
enum config_state
{
  Idle,
  Collecting,
  Experimenting
};

struct config_manager_base
{
  // This fn gets called before learning any example
  virtual void one_step(const multi_ex&) = 0;
  // This fn is responsible for applying a config
  // tracked by 'stride' into the example.
  // the impl is responsible of tracking this config-stride mapping
  virtual void apply_config(example*, size_t) = 0;
  // This fn is the 'undo' of configure_interactions
  virtual void revert_config(example*) = 0;
  virtual void save_load(io_buf&, bool, bool) = 0;
  virtual void persist(metric_sink&) = 0;
};

struct config_manager : config_manager_base
{
  config_state current_state = Idle;
  size_t county = 0;
  size_t current_champ = 0;
  const size_t budget;
  const size_t max_live_configs;
  quadratic_exclusion_oracle oc;

  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, size_t> ns_counter;

  // Stores all configs in consideration (Map allows easy deletion unlike vector)
  std::map<size_t, exclusion_config> configs;

  std::vector<scored_config> scores;

  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, size_t>> index_queue;

  config_manager(size_t, size_t);

  void one_step(const multi_ex&) override;
  void apply_config(example*, size_t) override;
  void revert_config(example*) override;
  void save_load(io_buf&, bool, bool) override;
  void persist(metric_sink&) override;

private:
  void update_champ();
  float calc_priority(size_t);
  void gen_exclusion_configs(const multi_ex&);
  bool repopulate_index_queue();
  void handle_empty_budget(scored_config&);
  void update_live_configs();
};

struct automl
{
  config_manager cm;
  vw* all;                              //  TBD might not be needed
  LEARNER::multi_learner* adf_learner;  //  re-use print from cb_explore_adf
  VW::version_struct model_file_version;
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
  automl(size_t starting_budget, VW::version_struct model_file_version, size_t max_live_configs)
      : cm(starting_budget, max_live_configs), model_file_version(model_file_version)
  {
  }
};

}  // namespace automl
}  // namespace VW
