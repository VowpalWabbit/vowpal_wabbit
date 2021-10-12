// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "rand_state.h"
#include "reductions_fwd.h"
#include "distributionally_robust.h"
#include "constant.h"  // NUM_NAMESPACES
#include "metric_sink.h"
#include "action_score.h"
#include "debug_log.h"
#include "reductions.h"
#include "learner.h"
#include "io/logger.h"
#include "vw.h"
#include "cache.h"
#include <map>
#include <set>
#include <queue>

using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace automl
{
using namespace_index = unsigned char;
using interaction_vec_t = std::vector<std::vector<namespace_index>>;

VW::LEARNER::base_learner* automl_setup(VW::setup_base_i&);

namespace details
{
void fail_if_enabled(vw&, const std::set<std::string>&);
// void print_weights_nonzero(vw*, size_t, dense_parameters&);
}  // namespace details

constexpr size_t MAX_CONFIGS = 10;
constexpr size_t CONGIGS_PER_CHAMP_CHANGE = 5;

struct scored_config
{
  VW::distributionally_robust::ChiSquared chisq;
  float ips = 0.0;
  float last_w = 0.0;
  float last_r = 0.0;
  size_t update_count = 0;
  size_t config_index = 0;
  bool eligible_to_inactivate = false;
  interaction_vec_t live_interactions;  // Live pre-allocated vectors in use

  scored_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}

  void update(float w, float r);
  void save_load(io_buf&, bool, bool);
  void persist(metric_sink&, const std::string&);
  float current_ips() const;
  void reset_stats();
};

// all possible states of exclusion config
enum class config_state
{
  New,
  Live,
  Inactive,
  Removed
};

struct exclusion_config
{
  std::map<namespace_index, std::set<namespace_index>> exclusions;
  size_t lease;
  float ips = 0.f;
  float lower_bound = std::numeric_limits<float>::infinity();
  config_state state = VW::automl::config_state::New;

  exclusion_config(size_t lease = 10) : lease(lease) {}

  void save_load(io_buf&, bool, bool);
};

// all possible states of automl
enum class automl_state
{
  Collecting,
  Experimenting
};

struct config_manager
{
  // This fn is responsible for applying a config
  // tracked by 'live_slot' into the example.
  // the impl is responsible of tracking this config-live_slot mapping
  void apply_config(example*, size_t);
  // This fn is the 'undo' of configure_interactions
  void revert_config(example*);
  void save_load(io_buf&, bool, bool);
  void persist(metric_sink&);

  // Public Chacha functions
  void config_oracle();
  void pre_process(const multi_ex&);
  void schedule();
  void update_champ();
};

using priority_func = float(const exclusion_config&, const std::map<namespace_index, size_t>&);

struct interaction_config_manager : config_manager
{
  size_t total_learn_count = 0;
  size_t current_champ = 0;
  size_t global_lease;
  size_t max_live_configs;
  uint64_t seed;
  rand_state random_state;
  size_t priority_challengers;

  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, size_t> ns_counter;

  // Stores all configs in consideration (Map allows easy deletion unlike vector)
  std::map<size_t, exclusion_config> configs;

  // Stores scores of live configs, size will never exceed max_live_configs
  std::vector<scored_config> scores;

  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, size_t>> index_queue;

  interaction_config_manager(
      size_t, size_t, uint64_t, size_t, float (*)(const exclusion_config&, const std::map<namespace_index, size_t>&));

  void apply_config(example*, size_t);
  void revert_config(example*);
  void save_load(io_buf&, bool, bool);
  void persist(metric_sink&);

  // Public Chacha functions
  void config_oracle();
  void pre_process(const multi_ex&);
  void schedule();
  void update_champ();

  // Public for save_load
  void gen_quadratic_interactions(size_t);

private:
  bool better(const exclusion_config&, const exclusion_config&) const;
  bool worse(const exclusion_config&, const exclusion_config&) const;
  size_t choose();
  priority_func* calc_priority;
  bool repopulate_index_queue();
  bool swap_eligible_to_inactivate(size_t);
};

template <typename CMType>
struct automl
{
  automl_state current_state = automl_state::Collecting;
  std::unique_ptr<CMType> cm;
  LEARNER::multi_learner* adf_learner = nullptr;  //  re-use print from cb_explore_adf
  automl(std::unique_ptr<CMType> cm) : cm(std::move(cm)) {}
  // This fn gets called before learning any example
  void one_step(multi_learner&, multi_ex&, CB::cb_class&, size_t);
  void offset_learn(multi_learner&, multi_ex&, CB::cb_class&, size_t);

private:
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
};

}  // namespace automl
}  // namespace VW
