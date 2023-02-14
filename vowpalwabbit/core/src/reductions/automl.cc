// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl.h"

#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/automl_impl.h"
#include "vw/core/estimators/confidence_sequence_robust.h"
#include "vw/core/multi_model_utils.h"

// TODO: delete this three includes
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <cfloat>
#include <iomanip>

using namespace VW::config;
using namespace VW::LEARNER;
using namespace VW::reductions::automl;

namespace
{
template <typename CMType, bool is_explore>
void predict_automl(automl<CMType>& data, learner& base, VW::multi_ex& ec)
{
  data.cm->process_example(ec);

  interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  auto restore_guard = VW::scope_exit(
      [&ec, &incoming_interactions]
      {
        for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
      });

  for (VW::example* ex : ec) { apply_config(ex, &data.cm->estimators[data.cm->current_champ].first.live_interactions); }
  base.predict(ec, data.cm->current_champ);
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(automl<CMType>& data, learner& base, VW::multi_ex& ec)
{
  VW::cb_class logged{};
  uint64_t labelled_action = 0;
  const auto it = std::find_if(ec.begin(), ec.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });

  if (it != ec.end())
  {
    logged = (*it)->l.cb.costs[0];
    labelled_action = std::distance(ec.begin(), it);
  }

  data.one_step(base, ec, logged, labelled_action);
  assert(ec[0]->interactions != nullptr);
}

template <typename CMType, bool verbose>
void persist(automl<CMType>& data, VW::metric_sink& metrics)
{
  if (verbose) { data.cm->persist(metrics, true); }
  else { data.cm->persist(metrics, false); }
}

template <typename CMType>
void pre_save_load_automl(VW::workspace& all, automl<CMType>& data)
{
  options_i& options = *all.options;
  if (!data.should_save_predict_only_model) { return; }
  // Clear non-champ weights first

  std::swap(*data.cm->_gd_normalized, data.cm->per_live_model_state_double[0]);
  std::swap(*data.cm->_gd_total_weight, data.cm->per_live_model_state_double[1]);
  std::swap(*data.cm->_sd_gravity, data.cm->per_live_model_state_double[2]);
  std::swap(*data.cm->_cb_adf_event_sum, data.cm->per_live_model_state_uint64[0]);
  std::swap(*data.cm->_cb_adf_action_sum, data.cm->per_live_model_state_uint64[1]);

  // Adjust champ weights to new single-model space
  VW::reductions::multi_model::reduce_innermost_model_weights(
      data.cm->weights, 0, data.cm->wpp, data.cm->max_live_configs);

  for (auto& group : options.get_all_option_group_definitions())
  {
    if (group.m_name == "[Reduction] Automl Options")
    {
      for (auto& opt : group.m_options) { opt->m_keep = false; }
    }
  }

  all.num_bits = all.num_bits - static_cast<uint32_t>(std::log2(data.cm->max_live_configs));
  options.get_typed_option<uint32_t>("bit_precision").value(all.num_bits);

  std::vector<std::string> interactions_opt;
  for (auto& interaction : data.cm->estimators[0].first.live_interactions)
  {
    std::string interaction_string;
    for (auto& ns : interaction)
    {
      if (ns == ' ') { interaction_string += "\\x20"; }
      else { interaction_string += ns; }
    }
    interactions_opt.push_back(interaction_string);
  }
  options.insert("interactions", "");
  options.get_typed_option<std::vector<std::string>>("interactions").value(interactions_opt);
}

template <typename CMType>
void save_load_automl(automl<CMType>& aml, VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, aml); }
  else if (!aml.should_save_predict_only_model) { VW::model_utils::write_model_field(io, aml, "_automl", text); }
}

// Basic implementation of scheduler to pick new configs when one runs out of lease.
// Highest priority will be picked first because of max-PQ implementation, this will
// be the config with the least exclusion. Note that all configs will run to lease
// before priorities and lease are reset.
float calc_priority_favor_popular_namespaces(
    const ns_based_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : config.elements) { priority -= ns_counter.at(*ns_pair.begin()); }
  return priority;
}

// Same as above, returns 0 (includes rest to remove unused variable warning)
float calc_priority_empty(const ns_based_config&, const std::map<VW::namespace_index, uint64_t>&) { return 0.f; }
}  // namespace

template <typename T, typename E>
std::shared_ptr<VW::LEARNER::learner> make_automl_with_impl(VW::setup_base_i& stack_builder,
    std::shared_ptr<VW::LEARNER::learner> base_learner, uint64_t max_live_configs, bool verbose_metrics,
    std::string& oracle_type, uint64_t default_lease, VW::workspace& all, int32_t priority_challengers,
    std::string& interaction_type, std::string& priority_type, float automl_significance_level, bool ccb_on,
    bool predict_only_model, bool reversed_learning_order, config_type conf_type, bool trace_logging,
    bool reward_as_cost, double tol_x, bool is_brentq)
{
  using config_manager_type = interaction_config_manager<T, E>;

  priority_func calc_priority;

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "favor_popular_namespaces") { calc_priority = &calc_priority_favor_popular_namespaces; }
  else { THROW("Invalid priority function provided"); }

  // Note that all.wpp will not be set correctly until after setup
  assert(oracle_type == "one_diff" || oracle_type == "rand" || oracle_type == "champdupe" ||
      oracle_type == "one_diff_inclusion" || oracle_type == "qbase_cubic");

  std::string trace_file_name_prefix = "";

  if (trace_logging)
  {
    auto t = std::time(nullptr);
    auto lc = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&lc, "%d%m.%H%M%S");
    trace_file_name_prefix = oss.str();
  }

  auto cm = VW::make_unique<config_manager_type>(default_lease, max_live_configs, all.get_random_state(),
      static_cast<uint64_t>(priority_challengers), interaction_type, oracle_type, all.weights.dense_weights,
      calc_priority, automl_significance_level, &all.logger, all.wpp, ccb_on, conf_type, trace_file_name_prefix,
      reward_as_cost, tol_x, is_brentq);
  auto data = VW::make_unique<automl<config_manager_type>>(
      std::move(cm), &all.logger, predict_only_model, trace_file_name_prefix);
  data->debug_reverse_learning_order = reversed_learning_order;
  data->cm->per_live_model_state_double = std::vector<double>(max_live_configs * 3, 0.f);
  data->cm->per_live_model_state_uint64 = std::vector<uint64_t>(max_live_configs * 2, 0.f);

  auto ppw = max_live_configs;
  auto* persist_ptr = verbose_metrics ? persist<config_manager_type, true> : persist<config_manager_type, false>;
  data->adf_learner = require_multiline(base_learner->get_learner_by_name_prefix("cb_adf"));
  VW::reductions::gd& gd = *static_cast<VW::reductions::gd*>(
      base_learner->get_learner_by_name_prefix("gd")->get_internal_type_erased_data_pointer_test_use_only());
  auto& adf_data =
      *static_cast<VW::reductions::cb_adf*>(data->adf_learner->get_internal_type_erased_data_pointer_test_use_only());
  data->cm->_gd_normalized = &(gd.per_model_states[0].normalized_sum_norm_x);
  data->cm->_gd_total_weight = &(gd.per_model_states[0].total_weight);
  data->cm->_cb_adf_event_sum = &(adf_data.gen_cs.event_sum);
  data->cm->_cb_adf_action_sum = &(adf_data.gen_cs.action_sum);
  data->cm->_sd_gravity = &(all.sd->gravity);

  auto l = make_reduction_learner(std::move(data), require_multiline(base_learner),
      learn_automl<config_manager_type, true>, predict_automl<config_manager_type, true>,
      stack_builder.get_setupfn_name(VW::reductions::automl_setup))
               .set_params_per_weight(ppw)  // refactor pm
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_save_load(save_load_automl)
               .set_persist_metrics(persist_ptr)
               .set_learn_returns_prediction(true)
               .set_pre_save_load(pre_save_load_automl)
               .build();
  return l;
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  uint64_t default_lease = 4000;
  uint64_t max_live_configs = 4;
  std::string priority_type = "none";
  int32_t priority_challengers = -1;
  bool verbose_metrics = false;
  std::string interaction_type = "quadratic";
  std::string oracle_type = "one_diff";
  float automl_significance_level = VW::details::CS_ROBUST_DEFAULT_ALPHA;
  bool reversed_learning_order = false;
  bool fixed_significance_level = false;
  bool trace_logging = false;
  bool reward_as_cost = false;
  float tol_x = 1e-6f;
  std::string opt_func = "bisect";

  option_group_definition new_options("[Reduction] Automl");
  new_options
      .add(make_option("automl", max_live_configs)
               .necessary()
               .keep()
               .default_value(4)
               .help("Set number of live configs")
               .experimental())
      .add(make_option("default_lease", default_lease)
               .keep()
               .allow_override()
               .default_value(4000)
               .help("Set initial lease for automl interactions")
               .experimental())
      .add(make_option("priority_type", priority_type)
               .keep()
               .allow_override()
               .default_value("none")
               .one_of({"none", "favor_popular_namespaces"})
               .help("Set function to determine next config")
               .experimental())
      .add(make_option("priority_challengers", priority_challengers)
               .keep()
               .allow_override()
               .default_value(-1)
               .help("Set number of priority challengers to use. Set to -1 to use half of total live configs.")
               .experimental())
      .add(make_option("verbose_metrics", verbose_metrics).help("Extended metrics for debugging").experimental())
      .add(make_option("csv_trace", trace_logging).help("Extended tracing for debugging").experimental())
      .add(make_option("interaction_type", interaction_type)
               .keep()
               .default_value("quadratic")
               .one_of({"quadratic", "cubic"})
               .help("Set what type of interactions to use")
               .experimental())
      .add(make_option("oracle_type", oracle_type)
               .keep()
               .allow_override()
               .default_value("one_diff")
               .one_of({"one_diff", "rand", "champdupe", "one_diff_inclusion", "qbase_cubic"})
               .help("Set oracle to generate configs")
               .experimental())
      .add(make_option("debug_reversed_learn", reversed_learning_order)
               .default_value(false)
               .help("Debug: learn each config in reversed order (last to first).")
               .experimental())
      .add(make_option("automl_significance_level", automl_significance_level)
               .keep()
               .default_value(VW::details::CS_ROBUST_DEFAULT_ALPHA)
               .allow_override()
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("fixed_significance_level", fixed_significance_level)
               .keep()
               .help("Use fixed significance level as opposed to scaling by model count (bonferroni correction)")
               .experimental())
      .add(make_option("reward_as_cost", reward_as_cost)
               .keep()
               .help("Treat rewards as cost (do not negate sign)")
               .experimental())
      .add(make_option("tol_x", tol_x)
               .default_value(1e-6f)
               .keep()
               .help("Tolerance for estimation optimization")
               .experimental())
      .add(make_option("opt_func", opt_func)
               .default_value("bisect")
               .keep()
               .one_of({"bisect", "brentq"})
               .help("Optimization function for estimation)")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (priority_challengers < 0) { priority_challengers = (static_cast<int>(max_live_configs) - 1) / 2; }

  if (!fixed_significance_level) { automl_significance_level /= max_live_configs; }

  bool is_brentq = opt_func == "brentq";
  bool ccb_on = options.was_supplied("ccb_explore_adf");
  bool predict_only_model = options.was_supplied("predict_only_model");

  if (max_live_configs > MAX_CONFIGS)
  {
    THROW("Maximum number of configs is "
        << MAX_CONFIGS << " and " << max_live_configs
        << " were specified. Please decrease the number of configs with the --automl flag.");
  }

  // override and clear all the global interactions
  // see parser.cc line 740
  all.interactions.clear();
  assert(all.interactions.empty() == true);

  // make sure we setup the rest of the stack with cleared interactions
  // to make sure there are not subtle bugs
  auto learner = stack_builder.setup_base_learner();

  assert(all.interactions.empty() == true);

  assert(all.weights.sparse == false);
  if (all.weights.sparse) THROW("--automl does not work with sparse weights");

  VW::reductions::util::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  config_type conf_type = (oracle_type == "one_diff_inclusion") ? config_type::Interaction : config_type::Exclusion;

  if (conf_type == config_type::Interaction && oracle_type == "rand")
  {
    THROW("--config_type interaction cannot be used with --oracle_type rand");
  }

  // only this has been tested
  if (learner->is_multiline())
  {
    if (oracle_type == "one_diff")
    {
      return make_automl_with_impl<config_oracle<one_diff_impl>, VW::estimators::confidence_sequence_robust>(
          stack_builder, learner, max_live_configs, verbose_metrics, oracle_type, default_lease, all,
          priority_challengers, interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type, trace_logging, reward_as_cost, tol_x, is_brentq);
    }
    else if (oracle_type == "rand")
    {
      return make_automl_with_impl<config_oracle<oracle_rand_impl>, VW::estimators::confidence_sequence_robust>(
          stack_builder, learner, max_live_configs, verbose_metrics, oracle_type, default_lease, all,
          priority_challengers, interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type, trace_logging, reward_as_cost, tol_x, is_brentq);
    }
    else if (oracle_type == "champdupe")
    {
      return make_automl_with_impl<config_oracle<champdupe_impl>, VW::estimators::confidence_sequence_robust>(
          stack_builder, learner, max_live_configs, verbose_metrics, oracle_type, default_lease, all,
          priority_challengers, interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type, trace_logging, reward_as_cost, tol_x, is_brentq);
    }
    else if (oracle_type == "one_diff_inclusion")
    {
      return make_automl_with_impl<config_oracle<one_diff_inclusion_impl>, VW::estimators::confidence_sequence_robust>(
          stack_builder, learner, max_live_configs, verbose_metrics, oracle_type, default_lease, all,
          priority_challengers, interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type, trace_logging, reward_as_cost, tol_x, is_brentq);
    }
    else if (oracle_type == "qbase_cubic")
    {
      interaction_type = "both";
      conf_type = config_type::Interaction;
      return make_automl_with_impl<config_oracle<qbase_cubic>, VW::estimators::confidence_sequence_robust>(
          stack_builder, learner, max_live_configs, verbose_metrics, oracle_type, default_lease, all,
          priority_challengers, interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type, trace_logging, reward_as_cost, tol_x, is_brentq);
    }
  }
  else
  {
    // not implemented yet
    THROW("fatal: automl not supported for single line learners yet");
  }
  return nullptr;
}
