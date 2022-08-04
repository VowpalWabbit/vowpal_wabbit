// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/array_parameters_dense.h"
#include "vw/core/estimator_config.h"
#include "vw/core/learner.h"
#include "vw/core/rand_state.h"

#include <queue>

using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace reductions
{
namespace automl
{
namespace
{
constexpr uint64_t MAX_CONFIGS = 10;
constexpr uint64_t CONFIGS_PER_CHAMP_CHANGE = 10;
}  // namespace

using interaction_vec_t = std::vector<std::vector<namespace_index>>;

struct aml_estimator : VW::estimator_config
{
  aml_estimator() : VW::estimator_config() {}
  aml_estimator(double alpha, double tau) : VW::estimator_config(alpha, tau) {}
  aml_estimator(
      VW::estimator_config sc, uint64_t config_index, bool eligible_to_inactivate, interaction_vec_t& live_interactions)
      : VW::estimator_config(sc)
  {
    this->config_index = config_index;
    this->eligible_to_inactivate = eligible_to_inactivate;
    this->live_interactions = live_interactions;
  }
  uint64_t config_index = 0;
  bool eligible_to_inactivate = false;
  interaction_vec_t live_interactions;  // Live pre-allocated vectors in use

  void persist(metric_sink&, const std::string&, bool, const std::string&);
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
  std::set<std::vector<namespace_index>> exclusions;
  uint64_t lease;
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
  void persist(metric_sink&, bool);
  // config managers own the underlaying weights so they need to know how to clear
  void clear_non_champ_weights();

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
  const uint64_t max_live_configs;
  std::shared_ptr<VW::rand_state> random_state;
  uint64_t priority_challengers;
  uint64_t valid_config_size = 0;
  std::string interaction_type;
  std::string oracle_type;
  dense_parameters& weights;
  priority_func* calc_priority;
  double automl_significance_level;
  double automl_estimator_decay;
  VW::io::logger* logger;
  uint32_t& wpp;
  bool lb_trick;

  // TODO: delete all this, gd and cb_adf must respect ft_offset
  std::vector<double> per_live_model_state_double;
  std::vector<uint64_t> per_live_model_state_uint64;
  double* _gd_normalized = nullptr;
  double* _gd_total_weight = nullptr;
  uint64_t* _cb_adf_event_sum = nullptr;
  uint64_t* _cb_adf_action_sum = nullptr;

  // Stores all namespaces currently seen -- Namespace switch could we use array, ask Jack
  std::map<namespace_index, uint64_t> ns_counter;

  // Stores all configs in consideration
  std::vector<exclusion_config> configs;

  // Stores estimators of live configs, size will never exceed max_live_configs. Each pair will be of the form
  // <challenger_estimator, champ_estimator> for the horizon of a given challenger. Thus each challenger has one
  // horizon and the champ has one horizon for each challenger
  std::vector<std::pair<aml_estimator, estimator_config>> estimators;

  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, uint64_t>> index_queue;

  interaction_config_manager(uint64_t, uint64_t, std::shared_ptr<VW::rand_state>, uint64_t, std::string, std::string,
      dense_parameters&, float (*)(const exclusion_config&, const std::map<namespace_index, uint64_t>&), double, double,
      VW::io::logger*, uint32_t&, bool);

  void apply_config(example*, uint64_t);
  void do_learning(multi_learner&, multi_ex&, uint64_t);
  void persist(metric_sink&, bool);

  // Public Chacha functions
  void config_oracle();
  void pre_process(const multi_ex&);
  void schedule();
  void update_champ();

  // Public for save_load
  void gen_interactions(uint64_t);
  void clear_non_champ_weights();

private:
  bool better(uint64_t live_slot);
  bool worse(uint64_t live_slot);
  uint64_t choose();
  bool repopulate_index_queue();
  bool swap_eligible_to_inactivate(uint64_t);
  void insert_config(std::set<std::vector<namespace_index>>&& new_exclusions, bool allow_dups = false);
};

template <typename CMType>
struct automl
{
  automl_state current_state = automl_state::Collecting;
  std::unique_ptr<CMType> cm;
  VW::io::logger* logger;
  LEARNER::multi_learner* adf_learner = nullptr;  //  re-use print from cb_explore_adf
  bool debug_reverse_learning_order = false;
  const bool should_save_predict_only_model;

  automl(std::unique_ptr<CMType> cm, VW::io::logger* logger, bool predict_only_model)
      : cm(std::move(cm)), logger(logger), should_save_predict_only_model(predict_only_model)
  {
  }
  // This fn gets called before learning any example
  // void one_step(multi_learner&, multi_ex&, CB::cb_class&, uint64_t);
  // template <typename CMType>
  void one_step(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
  {
    cm->total_learn_count++;
    switch (current_state)
    {
      case automl_state::Collecting:
        cm->pre_process(ec);
        cm->config_oracle();
        offset_learn(base, ec, logged, labelled_action);
        current_state = automl_state::Experimenting;
        break;

      case automl_state::Experimenting:
        cm->pre_process(ec);
        cm->schedule();
        offset_learn(base, ec, logged, labelled_action);
        cm->update_champ();
        break;

      default:
        break;
    }
  };
  // inner loop of learn driven by # MAX_CONFIGS
  void offset_learn(multi_learner& base, multi_ex& ec, CB::cb_class& logged, uint64_t labelled_action)
  {
    interaction_vec_t* incoming_interactions = ec[0]->interactions;
    for (VW::example* ex : ec)
    {
      _UNUSED(ex);
      assert(ex->interactions == incoming_interactions);
    }

    const float w = logged.probability > 0 ? 1 / logged.probability : 0;
    const float r = -logged.cost;

    int64_t live_slot = 0;
    int64_t current_champ = static_cast<int64_t>(cm->current_champ);
    assert(current_champ == 0);

    auto restore_guard = VW::scope_exit([&ec, &incoming_interactions]() {
      for (example* ex : ec) { ex->interactions = incoming_interactions; }
    });

    // Learn and update estimators of challengers
    for (int64_t current_slot_index = 1; static_cast<size_t>(current_slot_index) < cm->estimators.size();
         ++current_slot_index)
    {
      if (!debug_reverse_learning_order) { live_slot = current_slot_index; }
      else
      {
        live_slot = cm->estimators.size() - current_slot_index;
      }
      cm->do_learning(base, ec, live_slot);
      cm->estimators[live_slot].first.update(ec[0]->pred.a_s[0].action == labelled_action ? w : 0, r);
    }

    // ** Note: champ learning is done after to ensure correct feature count in gd **
    // Learn and get action of champ
    cm->do_learning(base, ec, current_champ);
    auto champ_action = ec[0]->pred.a_s[0].action;
    for (live_slot = 1; static_cast<size_t>(live_slot) < cm->estimators.size(); ++live_slot)
    {
      if (cm->lb_trick) { cm->estimators[live_slot].second.update(champ_action == labelled_action ? w : 0, 1 - r); }
      else
      {
        cm->estimators[live_slot].second.update(champ_action == labelled_action ? w : 0, r);
      }
    }
  };

private:
  ACTION_SCORE::action_scores buffer_a_s;  // a sequence of classes with scores.  Also used for probabilities.
};
}  // namespace automl

void fail_if_enabled(VW::workspace& all, const std::set<std::string>& not_compat);
std::string interaction_vec_t_to_string(
    const VW::reductions::automl::interaction_vec_t& interactions, const std::string& interaction_type);
std::string exclusions_to_string(const std::set<std::vector<VW::namespace_index>>& exclusions);

}  // namespace reductions
namespace model_utils
{
template <typename CMType>
size_t write_model_field(io_buf&, const VW::reductions::automl::automl<CMType>&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::reductions::automl::exclusion_config&);
size_t read_model_field(io_buf&, VW::reductions::automl::aml_estimator&);
size_t read_model_field(io_buf&, VW::reductions::automl::interaction_config_manager&);
template <typename CMType>
size_t read_model_field(io_buf&, VW::reductions::automl::automl<CMType>&);
size_t write_model_field(io_buf&, const VW::reductions::automl::exclusion_config&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::reductions::automl::aml_estimator&, const std::string&, bool);
size_t write_model_field(io_buf&, const VW::reductions::automl::interaction_config_manager&, const std::string&, bool);
}  // namespace model_utils
VW::string_view to_string(reductions::automl::automl_state state);
VW::string_view to_string(reductions::automl::config_state state);
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
