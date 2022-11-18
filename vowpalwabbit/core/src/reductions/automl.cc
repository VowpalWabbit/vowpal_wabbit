// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl.h"

#include "details/automl_impl.h"
#include "vw/config/options.h"
#include "vw/core/confidence_sequence.h"

// TODO: delete this three includes
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;
using namespace VW::reductions::automl;

namespace
{
template <typename CMType, bool is_explore>
void predict_automl(automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
{
  data.cm->process_example(ec);

  interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
    for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
  });

  for (VW::example* ex : ec) { apply_config(ex, &data.cm->estimators[data.cm->current_champ].first.live_interactions); }

  base.predict(ec, data.cm->current_champ);
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
{
  CB::cb_class logged{};
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
  else
  {
    data.cm->persist(metrics, false);
  }
}

template <typename CMType>
void finish_example(VW::workspace& all, automl<CMType>& data, VW::multi_ex& ec)
{
  interaction_vec_t* incoming_interactions = ec[0]->interactions;

  uint64_t champ_live_slot = data.cm->current_champ;
  for (VW::example* ex : ec) { apply_config(ex, &data.cm->estimators[champ_live_slot].first.live_interactions); }

  {
    auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
      for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
    });

    data.adf_learner->print_example(all, ec);
  }

  VW::finish_example(all, ec);
}

template <typename CMType>
void convert_to_single_model(VW::workspace& all, automl<CMType>& data)
{
  options_i& options = *all.options;
  if (!options.was_supplied("predict_only_model")) { return; }
  // Clear non-champ weights first

  std::swap(*data.cm->_gd_normalized, data.cm->per_live_model_state_double[0]);
  std::swap(*data.cm->_gd_total_weight, data.cm->per_live_model_state_double[1]);
  std::swap(*data.cm->_sd_gravity, data.cm->per_live_model_state_double[2]);
  std::swap(*data.cm->_cb_adf_event_sum, data.cm->per_live_model_state_uint64[0]);
  std::swap(*data.cm->_cb_adf_action_sum, data.cm->per_live_model_state_uint64[1]);

  clear_non_champ_weights(data.cm->weights, data.cm->estimators.size(), data.cm->wpp);

  uint64_t multiplier = static_cast<uint64_t>(data.cm->wpp) << data.cm->weights.stride_shift();
  for (uint32_t index = 0; index < data.cm->weights.mask(); index += multiplier)
  {
    uint32_t cb_ind = index / data.cm->wpp;
    for (uint32_t stride = 0; stride < (static_cast<uint32_t>(1) << data.cm->weights.stride_shift()); ++stride)
    {
      if (data.cm->weights[index + stride] != 0.0f)
      { std::swap(data.cm->weights[index + stride], data.cm->weights[cb_ind + stride]); }
    }
  }
  uint32_t num_bits =
      options.was_supplied("bit_precision") ? options.get_typed_option<uint32_t>("bit_precision").value() : 18;
  std::set<std::string> aml_opts = {"automl", "global_lease", "cm_type", "priority_type", "priority_challengers",
      "verbose_metrics", "interaction_type", "oracle_type", "debug_reversed_learn", "automl_significance_level",
      "fixed_significance_level"};

  for (auto opt : aml_opts)
  {
    if (options.was_supplied(opt)) { options.remove_option(opt); }
  }

  options.get_typed_option<uint32_t>("bit_precision").value(num_bits - static_cast<uint32_t>(std::log2(data.cm->wpp)));
  all.num_bits = num_bits - static_cast<uint32_t>(std::log2(data.cm->wpp));

  std::vector<std::string> interactions_opt;
  for (auto& interaction : data.cm->estimators[0].first.live_interactions)
  {
    std::string interaction_string;
    for (auto& ns : interaction)
    {
      if (ns == ' ') { interaction_string += "\\x20"; }
      else
      {
        interaction_string += ns;
      }
    }
    interactions_opt.push_back(interaction_string);
  }
  options.insert("interactions","");
  options.get_typed_option<std::vector<std::string>>("interactions").value(interactions_opt);
}

template <typename CMType>
void save_load_aml(automl<CMType>& aml, io_buf& io, bool read, bool text)
{
  if (!aml.should_save_predict_only_model)
  {
    if (io.num_files() == 0) { return; }
    if (read) { VW::model_utils::read_model_field(io, aml); }
    else
    {
      VW::model_utils::write_model_field(io, aml, "_automl", text);
    }
  }
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
float calc_priority_empty(const ns_based_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  _UNUSED(config);
  _UNUSED(ns_counter);
  return 0.f;
}
}  // namespace

template <typename T, typename E>
VW::LEARNER::base_learner* make_automl_with_impl(VW::setup_base_i& stack_builder,
    VW::LEARNER::base_learner* base_learner, uint64_t max_live_configs, bool verbose_metrics, std::string& oracle_type,
    uint64_t global_lease, VW::workspace& all, int32_t priority_challengers, std::string& interaction_type,
    std::string& priority_type, float automl_significance_level, bool ccb_on, bool predict_only_model,
    bool reversed_learning_order, config_type conf_type)
{
  using config_manager_type = interaction_config_manager<T, E>;

  priority_func* calc_priority;

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "favor_popular_namespaces")
  {
    calc_priority = &calc_priority_favor_popular_namespaces;
  }
  else
  {
    THROW("Invalid priority function provided");
  }

  // Note that all.wpp will not be set correctly until after setup
  assert(oracle_type == "one_diff" || oracle_type == "rand" || oracle_type == "champdupe" ||
      oracle_type == "one_diff_inclusion");
  auto cm = VW::make_unique<config_manager_type>(global_lease, max_live_configs, all.get_random_state(),
      static_cast<uint64_t>(priority_challengers), interaction_type, oracle_type, all.weights.dense_weights,
      calc_priority, automl_significance_level, &all.logger, all.wpp, ccb_on, conf_type);
  auto data = VW::make_unique<automl<config_manager_type>>(std::move(cm), &all.logger, predict_only_model);
  data->debug_reverse_learning_order = reversed_learning_order;
  data->cm->per_live_model_state_double = std::vector<double>(max_live_configs * 3, 0.f);
  data->cm->per_live_model_state_uint64 = std::vector<uint64_t>(max_live_configs * 2, 0.f);

  auto ppw = max_live_configs;
  auto* persist_ptr = verbose_metrics ? persist<config_manager_type, true> : persist<config_manager_type, false>;
  data->adf_learner = as_multiline(base_learner->get_learner_by_name_prefix("cb_adf"));
  GD::gd& gd = *static_cast<GD::gd*>(
      base_learner->get_learner_by_name_prefix("gd")->get_internal_type_erased_data_pointer_test_use_only());
  auto& adf_data =
      *static_cast<CB_ADF::cb_adf*>(data->adf_learner->get_internal_type_erased_data_pointer_test_use_only());
  data->cm->_gd_normalized = &(gd.per_model_states[0].normalized_sum_norm_x);
  data->cm->_gd_total_weight = &(gd.per_model_states[0].total_weight);
  data->cm->_cb_adf_event_sum = &(adf_data.gen_cs.event_sum);
  data->cm->_cb_adf_action_sum = &(adf_data.gen_cs.action_sum);
  data->cm->_sd_gravity = &(all.sd->gravity);

  auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner), learn_automl<config_manager_type, true>,
      predict_automl<config_manager_type, true>,
      stack_builder.get_setupfn_name(VW::reductions::automl_setup))
                .set_params_per_weight(ppw)  // refactor pm
                .set_output_prediction_type(VW::prediction_type_t::ACTION_SCORES)
                .set_input_label_type(VW::label_type_t::CB)
                .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
                .set_output_label_type(VW::label_type_t::CB)
                .set_finish_example(::finish_example<config_manager_type>)
                .set_save_load(save_load_aml<config_manager_type>)
                .set_persist_metrics(persist_ptr)
                .set_output_prediction_type(base_learner->get_output_prediction_type())
                .set_learn_returns_prediction(true)
                .set_convert_to_single_model(::convert_to_single_model<config_manager_type>)
                .build();
  return make_base(*l);
}

VW::LEARNER::base_learner* VW::reductions::automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  uint64_t global_lease = 4000;
  uint64_t max_live_configs = 4;
  std::string cm_type = "interaction";
  std::string priority_type = "none";
  int32_t priority_challengers = -1;
  bool verbose_metrics = false;
  std::string interaction_type = "quadratic";
  std::string oracle_type = "one_diff";
  float automl_significance_level = CS_DEFAULT_ALPHA;
  bool reversed_learning_order = false;
  bool fixed_significance_level = false;
  option_group_definition new_options("[Reduction] Automl");
  new_options
      .add(make_option("automl", max_live_configs)
               .necessary()
               .keep()
               .default_value(4)
               .help("Set number of live configs")
               .experimental())
      .add(make_option("global_lease", global_lease)
               .keep()
               .allow_override()
               .default_value(4000)
               .help("Set initial lease for automl interactions")
               .experimental())
      .add(make_option("cm_type", cm_type)
               .keep()
               .default_value("interaction")
               .one_of({"interaction"})
               .help("Set type of config manager")
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
               .help("Set number of priority challengers to use")
               .experimental())
      .add(make_option("verbose_metrics", verbose_metrics).help("Extended metrics for debugging").experimental())
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
               .one_of({"one_diff", "rand", "champdupe", "one_diff_inclusion"})
               .help("Set oracle to generate configs")
               .experimental())
      .add(make_option("debug_reversed_learn", reversed_learning_order)
               .default_value(false)
               .help("Debug: learn each config in reversed order (last to first).")
               .experimental())
      .add(make_option("automl_significance_level", automl_significance_level)
               .keep()
               .default_value(CS_DEFAULT_ALPHA)
               .allow_override()
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("fixed_significance_level", fixed_significance_level)
               .keep()
               .help("Use fixed significance level as opposed to scaling by model count (bonferroni correction)")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (priority_challengers < 0) { priority_challengers = (static_cast<int>(max_live_configs) - 1) / 2; }

  if (!fixed_significance_level) { automl_significance_level /= max_live_configs; }

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
  auto* base_learner = stack_builder.setup_base_learner();

  assert(all.interactions.empty() == true);

  assert(all.weights.sparse == false);
  if (all.weights.sparse) THROW("--automl does not work with sparse weights");

  VW::reductions::util::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  config_type conf_type = (oracle_type == "one_diff_inclusion") ? config_type::Interaction : config_type::Exclusion;

  if (conf_type == config_type::Interaction && oracle_type == "rand")
  { THROW("--config_type interaction cannot be used with --oracle_type rand"); }

  // only this has been tested
  if (base_learner->is_multiline())
  {
    if (oracle_type == "one_diff")
    {
      return make_automl_with_impl<config_oracle<one_diff_impl>, VW::confidence_sequence>(stack_builder, base_learner,
          max_live_configs, verbose_metrics, oracle_type, global_lease, all, priority_challengers, interaction_type,
          priority_type, automl_significance_level, ccb_on, predict_only_model, reversed_learning_order, conf_type);
    }
    else if (oracle_type == "rand")
    {
      return make_automl_with_impl<config_oracle<oracle_rand_impl>, VW::confidence_sequence>(stack_builder,
          base_learner, max_live_configs, verbose_metrics, oracle_type, global_lease, all, priority_challengers,
          interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type);
    }
    else if (oracle_type == "champdupe")
    {
      return make_automl_with_impl<config_oracle<champdupe_impl>, VW::confidence_sequence>(stack_builder, base_learner,
          max_live_configs, verbose_metrics, oracle_type, global_lease, all, priority_challengers, interaction_type,
          priority_type, automl_significance_level, ccb_on, predict_only_model, reversed_learning_order, conf_type);
    }
    else if (oracle_type == "one_diff_inclusion")
    {
      return make_automl_with_impl<config_oracle<one_diff_inclusion_impl>, VW::confidence_sequence>(stack_builder,
          base_learner, max_live_configs, verbose_metrics, oracle_type, global_lease, all, priority_challengers,
          interaction_type, priority_type, automl_significance_level, ccb_on, predict_only_model,
          reversed_learning_order, conf_type);
    }
  }
  else
  {
    // not implemented yet
    THROW("fatal: automl not supported for single line learners yet");
  }
  return nullptr;
}
