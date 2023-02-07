// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/random.h"
#include "vw/core/array_parameters_dense.h"
#include "vw/core/learner.h"

#include <fstream>
#include <functional>
#include <queue>

namespace VW
{
namespace reductions
{
namespace automl
{
namespace
{
constexpr uint64_t MAX_CONFIGS = 129;
}  // namespace

using interaction_vec_t = std::vector<std::vector<namespace_index>>;

template <typename estimator_impl>
class aml_estimator
{
public:
  estimator_impl _estimator;
  aml_estimator() : _estimator(estimator_impl()) {}
  aml_estimator(double tol_x, bool is_brentq, double alpha) : _estimator(estimator_impl(tol_x, is_brentq, alpha)) {}
  aml_estimator(
      estimator_impl sc, uint64_t config_index, bool eligible_to_inactivate, interaction_vec_t& live_interactions)
      : _estimator(sc)
  {
    this->config_index = config_index;
    this->eligible_to_inactivate = eligible_to_inactivate;
    this->live_interactions = live_interactions;
  }
  uint64_t config_index = 0;
  bool eligible_to_inactivate = false;
  interaction_vec_t live_interactions;  // Live pre-allocated vectors in use

  void persist(metric_sink&, const std::string&, bool);
  static bool better(estimator_impl& challenger, estimator_impl& other)
  {
    return challenger.lower_bound() > other.upper_bound();
  }
};

template <typename estimator_impl>
using estimator_vec_t = std::vector<std::pair<aml_estimator<estimator_impl>, estimator_impl>>;

// all possible states of exclusion config
enum class config_state
{
  New,
  Live,
  Inactive,
  Removed
};

enum class config_type
{
  Exclusion,
  Interaction,
};

using set_ns_list_t = std::set<std::vector<VW::namespace_index>>;

class ns_based_config
{
public:
  set_ns_list_t elements;
  uint64_t lease;
  config_state state = VW::reductions::automl::config_state::New;
  config_type conf_type = VW::reductions::automl::config_type::Exclusion;

  ns_based_config(uint64_t lease = 10) : lease(lease) {}
  ns_based_config(set_ns_list_t&& ns_list, uint64_t lease, config_type conf_type)
      : elements(std::move(ns_list)), lease(lease), conf_type(conf_type)
  {
    this->state = VW::reductions::automl::config_state::New;
  }
  void reset(set_ns_list_t&& ns_list, uint64_t lease, config_type conf_type)
  {
    this->elements = std::move(ns_list);
    this->lease = lease;
    this->state = VW::reductions::automl::config_state::New;
    this->conf_type = conf_type;
  }
  static interaction_vec_t gen_quadratic_interactions(
      const std::map<namespace_index, uint64_t>& ns_counter, const set_ns_list_t& exclusions);
  static interaction_vec_t gen_cubic_interactions(
      const std::map<namespace_index, uint64_t>& ns_counter, const set_ns_list_t& exclusions);
  static void apply_config_to_interactions(const bool ccb_on, const std::map<namespace_index, uint64_t>& ns_counter,
      const std::string& interaction_type, const ns_based_config& config, interaction_vec_t& interactions);
};

using priority_func = std::function<float(const ns_based_config&, const std::map<namespace_index, uint64_t>&)>;

template <typename oracle_impl>
class config_oracle
{
public:
  std::string _interaction_type;
  const std::string _oracle_type;
  config_type _conf_type;

  // Maybe not needed with oracle, maps priority to config index, unused configs
  std::priority_queue<std::pair<float, uint64_t>> index_queue;
  // Stores all configs in consideration
  std::vector<ns_based_config> configs;

  priority_func calc_priority;
  const uint64_t default_lease;
  uint64_t valid_config_size = 0;
  oracle_impl _impl;

  config_oracle(uint64_t default_lease, priority_func calc_priority, const std::string& interaction_type,
      const std::string& oracle_type, std::shared_ptr<VW::rand_state>& rand_state, config_type conf_type);

  void gen_configs(const interaction_vec_t& champ_interactions, const std::map<namespace_index, uint64_t>& ns_counter);
  bool insert_config(set_ns_list_t&& new_elements, const std::map<namespace_index, uint64_t>& ns_counter,
      VW::reductions::automl::config_type conf_type, bool allow_dups = false);
  bool repopulate_index_queue(const std::map<namespace_index, uint64_t>& ns_counter);
  void insert_starting_configuration();
  void keep_best_two(uint64_t winner_config_index);
  static uint64_t choose(std::priority_queue<std::pair<float, uint64_t>>& index_queue);
};

class Iterator
{
public:
  using iterator_category = std::forward_iterator_tag;

  Iterator(size_t start_value = 0) : current(start_value) {}

  size_t operator*() const { return current; }

  Iterator& operator++()
  {
    current++;
    return *this;
  }

  bool operator==(const Iterator& rhs) const { return current == rhs.current; };
  bool operator<(const Iterator& rhs) const { return current < rhs.current; };

private:
  size_t current;
};

class oracle_rand_impl
{
public:
  size_t last_seen_ns_count = 0;
  VW::reductions::automl::interaction_vec_t total_space;
  std::shared_ptr<VW::rand_state> random_state;
  oracle_rand_impl(std::shared_ptr<VW::rand_state> random_state) : random_state(std::move(random_state)) {}
  void gen_ns_groupings_at(const interaction_vec_t& all_interactions, const size_t num, set_ns_list_t& copy_champ);
};
class one_diff_impl
{
public:
  void gen_ns_groupings_at(const interaction_vec_t& champ_interactions, const size_t num,
      set_ns_list_t::iterator& exclusion, const set_ns_list_t::iterator& exclusion_end, set_ns_list_t& new_elements);
  Iterator begin() { return Iterator(); }
  Iterator end(const interaction_vec_t& champ_interactions, const set_ns_list_t& champ_exclusions)
  {
    return Iterator(champ_interactions.size() + champ_exclusions.size());
  }
};
class champdupe_impl
{
public:
  Iterator begin() { return Iterator(); }
  Iterator end() { return Iterator(2); }
};

class one_diff_inclusion_impl
{
public:
  void gen_ns_groupings_at(const interaction_vec_t& champ_interactions, const size_t num, set_ns_list_t& copy_champ);
  Iterator begin() { return Iterator(); }
  Iterator end(const interaction_vec_t& all_interactions) { return Iterator(all_interactions.size()); }
};

class qbase_cubic
{
public:
  size_t last_seen_ns_count = 0;
  VW::reductions::automl::interaction_vec_t total_space;
  std::shared_ptr<VW::rand_state> random_state;
  qbase_cubic(std::shared_ptr<VW::rand_state> random_state) : random_state(std::move(random_state)) {}
  void gen_ns_groupings_at(const interaction_vec_t& all_interactions, const size_t num, set_ns_list_t& copy_champ);
};

template <typename config_oracle_impl, typename estimator_impl>
class interaction_config_manager
{
public:
  uint64_t total_champ_switches = 0;
  uint64_t total_learn_count = 0;
  const uint64_t current_champ = 0;
  const uint64_t default_lease;
  const uint64_t max_live_configs;
  uint64_t priority_challengers;
  dense_parameters& weights;
  double automl_significance_level;
  VW::io::logger* logger;
  uint32_t& wpp;
  const bool _ccb_on;
  config_oracle_impl _config_oracle;
  bool reward_as_cost;
  double tol_x;
  bool is_brentq;

  // TODO: delete all this, gd and cb_adf must respect ft_offset, see header import of automl.cc
  std::vector<double> per_live_model_state_double;
  std::vector<uint64_t> per_live_model_state_uint64;
  double* _gd_normalized = nullptr;
  double* _gd_total_weight = nullptr;
  double* _sd_gravity = nullptr;
  uint64_t* _cb_adf_event_sum = nullptr;
  uint64_t* _cb_adf_action_sum = nullptr;

  // Stores all namespaces currently seen
  std::map<namespace_index, uint64_t> ns_counter;

  // Stores estimators of live configs, size will never exceed max_live_configs. Each pair will be of the form
  // <challenger_estimator, champ_estimator> for the horizon of a given challenger. Thus each challenger has one
  // horizon and the champ has one horizon for each challenger
  estimator_vec_t<estimator_impl> estimators;
  std::unique_ptr<std::ofstream> champ_log_file;
  std::unique_ptr<std::ofstream> inputlabel_log_file;

  interaction_config_manager(uint64_t global_lease, uint64_t max_live_configs,
      std::shared_ptr<VW::rand_state> rand_state, uint64_t priority_challengers, const std::string& interaction_type,
      const std::string& oracle_type, dense_parameters& weights, priority_func calc_priority,
      double automl_significance_level, VW::io::logger* logger, uint32_t& wpp, bool ccb_on, config_type conf_type,
      std::string trace_prefix, bool reward_as_cost, double tol_x, bool is_brentq);

  void do_learning(VW::LEARNER::learner& base, multi_ex& ec, uint64_t live_slot);
  void persist(metric_sink& metrics, bool verbose);

  // Public Chacha functions
  void schedule();
  void check_for_new_champ();
  void process_example(const multi_ex& ec);
  static void apply_config_at_slot(estimator_vec_t<estimator_impl>& estimators, std::vector<ns_based_config>& configs,
      const uint64_t live_slot, const uint64_t config_index, const double sig_level, const double tol_x, bool is_brentq,
      const uint64_t priority_challengers);
  static void apply_new_champ(config_oracle_impl& config_oracle, const uint64_t winning_challenger_slot,
      estimator_vec_t<estimator_impl>& estimators, const uint64_t priority_challengers,
      const std::map<namespace_index, uint64_t>& ns_counter);
  static void insert_starting_configuration(estimator_vec_t<estimator_impl>& estimators,
      config_oracle_impl& config_oracle, const double sig_level, const double tol_x, bool is_brentq);

private:
  static bool swap_eligible_to_inactivate(estimator_vec_t<estimator_impl>& estimators, uint64_t);
};

bool count_namespaces(const multi_ex& ecs, std::map<namespace_index, uint64_t>& ns_counter);
void apply_config(example* ec, interaction_vec_t* live_interactions);
bool is_allowed_to_remove(const namespace_index ns);
void clear_non_champ_weights(dense_parameters& weights, uint32_t total, uint32_t& wpp);
bool worse();

// all possible states of automl
enum class automl_state
{
  Experimenting
};

template <typename CMType>
class automl
{
public:
  automl_state current_state = automl_state::Experimenting;
  std::unique_ptr<CMType> cm;
  VW::io::logger* logger;
  LEARNER::learner* adf_learner = nullptr;  //  re-use print from cb_explore_adf
  bool debug_reverse_learning_order = false;
  const bool should_save_predict_only_model;
  std::unique_ptr<std::ofstream> log_file;

  automl(std::unique_ptr<CMType> cm, VW::io::logger* logger, bool predict_only_model, std::string trace_prefix)
      : cm(std::move(cm)), logger(logger), should_save_predict_only_model(predict_only_model)
  {
    if (trace_prefix != "")
    {
      log_file = VW::make_unique<std::ofstream>(trace_prefix + ".automl.cs.csv");
      *log_file << "example_count, slot_id, champ_switch_count, lower_bound, upper_bound, champ_lower_bound, "
                   "champ_upper_bound"
                << std::endl;
    }
  }
  // This fn gets called before learning any example
  void one_step(VW::LEARNER::learner& base, multi_ex& ec, VW::cb_class& logged, uint64_t labelled_action);
  void offset_learn(VW::LEARNER::learner& base, multi_ex& ec, VW::cb_class& logged, uint64_t labelled_action);
};
}  // namespace automl

namespace util
{
void fail_if_enabled(VW::workspace& all, const std::set<std::string>& not_compat);
std::string interaction_vec_t_to_string(const VW::reductions::automl::interaction_vec_t& interactions);
std::string elements_to_string(const automl::set_ns_list_t& elements, const char* const delim = ", ");
}  // namespace util
}  // namespace reductions
namespace model_utils
{
template <typename CMType>
size_t write_model_field(io_buf&, const VW::reductions::automl::automl<CMType>&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::reductions::automl::ns_based_config&);
template <typename estimator_impl>
size_t read_model_field(io_buf&, VW::reductions::automl::aml_estimator<estimator_impl>&);
template <typename config_oracle_impl, typename estimator_impl>
size_t read_model_field(
    io_buf&, VW::reductions::automl::interaction_config_manager<config_oracle_impl, estimator_impl>&);
template <typename CMType>
size_t read_model_field(io_buf&, VW::reductions::automl::automl<CMType>&);
size_t write_model_field(io_buf&, const VW::reductions::automl::ns_based_config&, const std::string&, bool);
template <typename estimator_impl>
size_t write_model_field(
    io_buf&, const VW::reductions::automl::aml_estimator<estimator_impl>&, const std::string&, bool);
template <typename config_oracle_impl, typename estimator_impl>
size_t write_model_field(io_buf&,
    const VW::reductions::automl::interaction_config_manager<config_oracle_impl, estimator_impl>&, const std::string&,
    bool);
}  // namespace model_utils
VW::string_view to_string(reductions::automl::automl_state state);
VW::string_view to_string(reductions::automl::config_state state);
VW::string_view to_string(reductions::automl::config_type state);
}  // namespace VW

namespace fmt
{
template <>
class formatter<VW::reductions::automl::automl_state> : public formatter<std::string>
{
public:
  auto format(VW::reductions::automl::automl_state c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};

template <>
class formatter<VW::reductions::automl::config_state> : public formatter<std::string>
{
public:
  auto format(VW::reductions::automl::config_state c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};

template <>
class formatter<VW::reductions::automl::config_type> : public formatter<std::string>
{
public:
  auto format(VW::reductions::automl::config_type c, format_context& ctx) -> decltype(ctx.out())
  {
    return formatter<std::string>::format(std::string{VW::to_string(c)}, ctx);
  }
};
}  // namespace fmt
