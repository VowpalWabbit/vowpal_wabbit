// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/action_score.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/distributionally_robust.h"
#include "vw/core/learner.h"
#include "vw/core/metric_sink.h"
#include "vw/core/rand_state.h"
#include "vw/core/scored_config.h"
#include "vw/common/string_view.h"
#include "vw/core/vw_fwd.h"

#include <fmt/format.h>

#include <map>
#include <memory>
#include <queue>
#include <set>

using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* automl_setup(VW::setup_base_i&);

namespace automl
{
using interaction_vec_t = std::vector<std::vector<namespace_index>>;

struct aml_score : VW::scored_config
{
  aml_score() : VW::scored_config() {}
  aml_score(double alpha, double tau) : VW::scored_config(alpha, tau) {}
  uint64_t config_index = 0;
  bool eligible_to_inactivate = false;
  interaction_vec_t live_interactions;  // Live pre-allocated vectors in use

  void persist(metric_sink&, const std::string&, bool);
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
  uint64_t lease;
  float ips = 0.f;
  float lower_bound = std::numeric_limits<float>::infinity();
  config_state state = VW::reductions::automl::config_state::New;

  exclusion_config(uint64_t lease = 10) : lease(lease) {}
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
  void apply_config(example*, uint64_t);
  // This fn is the 'undo' of configure_interactions
  void revert_config(example*);
  void persist(metric_sink&, bool);

  // Public Chacha functions
  void config_oracle();
  void pre_process(const multi_ex&);
  void schedule();
  void update_champ();
};

using priority_func = float(const exclusion_config&, const std::map<namespace_index, uint64_t>&);

struct interaction_config_manager : config_manager
{
  uint64_t total_champ_switches = 0;
  uint64_t total_learn_count = 0;
  uint64_t current_champ = 0;
  uint64_t global_lease;
  uint64_t max_live_configs;
  std::shared_ptr<VW::rand_state> random_state;
  uint64_t priority_challengers;
  uint64_t valid_config_size = 0;
  bool keep_configs;
  std::string oracle_type;
  dense_parameters& weights;
  priority_func* calc_priority;
  double automl_alpha;
  double automl_tau;

  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, uint64_t> ns_counter;

  // Stores all configs in consideration (Map allows easy deletion unlike vector)
  std::map<uint64_t, exclusion_config> configs;

  // Stores scores of live configs, size will never exceed max_live_configs
  std::vector<aml_score> scores;

  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, uint64_t>> index_queue;

  interaction_config_manager(uint64_t, uint64_t, std::shared_ptr<VW::rand_state>, uint64_t, bool, std::string,
      dense_parameters&, float (*)(const exclusion_config&, const std::map<namespace_index, uint64_t>&), double,
      double);

  void apply_config(example*, uint64_t);
  void revert_config(example*);
  void persist(metric_sink&, bool);

  // Public Chacha functions
  void config_oracle();
  void pre_process(const multi_ex&);
  void schedule();
  void update_champ();

  // Public for save_load
  void gen_quadratic_interactions(uint64_t);

private:
  bool better(const exclusion_config&, const exclusion_config&) const;
  bool worse(const exclusion_config&, const exclusion_config&) const;
  uint64_t choose();
  bool repopulate_index_queue();
  bool swap_eligible_to_inactivate(uint64_t);
  void insert_config(std::map<namespace_index, std::set<namespace_index>>&&);
};

template <typename CMType>
struct automl
{
  automl_state current_state = automl_state::Collecting;
  std::unique_ptr<CMType> cm;
  LEARNER::multi_learner* adf_learner = nullptr;  //  re-use print from cb_explore_adf
  automl(std::unique_ptr<CMType> cm) : cm(std::move(cm)) {}
  // This fn gets called before learning any example
  void one_step(multi_learner&, multi_ex&, CB::cb_class&, uint64_t);
  void offset_learn(multi_learner&, multi_ex&, CB::cb_class&, uint64_t);

private:
  ACTION_SCORE::action_scores champ_a_s;  // a sequence of classes with scores.  Also used for probabilities.
};

}  // namespace automl
}  // namespace reductions

VW::string_view to_string(reductions::automl::automl_state state);
VW::string_view to_string(reductions::automl::config_state state);

namespace model_utils
{
template <typename CMType>
size_t write_model_field(io_buf&, const VW::reductions::automl::automl<CMType>&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::reductions::automl::exclusion_config&);
size_t read_model_field(io_buf&, VW::reductions::automl::aml_score&);
size_t read_model_field(io_buf&, VW::reductions::automl::interaction_config_manager&);
template <typename CMType>
size_t read_model_field(io_buf&, VW::reductions::automl::automl<CMType>&);
size_t write_model_field(io_buf&, const VW::reductions::automl::exclusion_config&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::reductions::automl::aml_score&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::reductions::automl::interaction_config_manager&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW

namespace fmt
{
template <>
struct formatter<VW::reductions::automl::automl_state> : formatter<std::string>
{
  auto format(VW::reductions::automl::automl_state c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};

template <>
struct formatter<VW::reductions::automl::config_state> : formatter<std::string>
{
  auto format(VW::reductions::automl::config_state c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};
}  // namespace fmt