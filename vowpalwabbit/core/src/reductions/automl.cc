// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/automl/automl.h"

#include "vw/config/options.h"
#include "vw/core/reductions/automl/automl_impl.h"
#include "vw/core/reductions/automl/automl_iomodel.h"

// TODO: delete this two includes
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/gd.h"

#include <cfloat>

using namespace VW::config;
using namespace VW::LEARNER;

namespace
{
template <typename CMType, bool is_explore>
void predict_automl(VW::reductions::automl::automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
{
  VW::reductions::automl::interaction_vec_t* incoming_interactions = ec[0]->interactions;
  for (VW::example* ex : ec)
  {
    _UNUSED(ex);
    assert(ex->interactions == incoming_interactions);
  }

  // if (ec[0]->interactions != nullptr)
  // { data.logger->out_info("pred: incoming interaction: {}", ::interaction_vec_t_to_string(*(ec[0]->interactions))); }
  uint64_t champ_live_slot = data.cm->current_champ;
  for (VW::example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

  auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
    for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
  });

  base.predict(ec, champ_live_slot);
}

// this is the registered learn function for this reduction
// mostly uses config_manager and actual_learn(..)
template <typename CMType, bool is_explore>
void learn_automl(VW::reductions::automl::automl<CMType>& data, multi_learner& base, VW::multi_ex& ec)
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
void persist(VW::reductions::automl::automl<CMType>& data, VW::metric_sink& metrics)
{
  if (verbose) { data.cm->persist(metrics, true); }
  else
  {
    data.cm->persist(metrics, false);
  }
}

template <typename CMType>
void finish_example(VW::workspace& all, VW::reductions::automl::automl<CMType>& data, VW::multi_ex& ec)
{
  VW::reductions::automl::interaction_vec_t* incoming_interactions = ec[0]->interactions;

  uint64_t champ_live_slot = data.cm->current_champ;
  for (VW::example* ex : ec) { data.cm->apply_config(ex, champ_live_slot); }

  {
    auto restore_guard = VW::scope_exit([&ec, &incoming_interactions] {
      for (VW::example* ex : ec) { ex->interactions = incoming_interactions; }
    });

    data.adf_learner->print_example(all, ec);
  }

  VW::finish_example(all, ec);
}

template <typename CMType>
void save_load_aml(VW::reductions::automl::automl<CMType>& aml, io_buf& io, bool read, bool text)
{
  if (aml.should_save_predict_only_model) { aml.cm->clear_non_champ_weights(); }
  if (io.num_files() == 0) { return; }
  if (read) { VW::model_utils::read_model_field(io, aml); }
  else
  {
    VW::model_utils::write_model_field(io, aml, "_automl", text);
  }
}

// Basic implementation of scheduler to pick new configs when one runs out of lease.
// Highest priority will be picked first because of max-PQ implementation, this will
// be the config with the least exclusion. Note that all configs will run to lease
// before priorities and lease are reset.
float calc_priority_favor_popular_namespaces(
    const VW::reductions::automl::exclusion_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  float priority = 0.f;
  for (const auto& ns_pair : config.exclusions) { priority -= ns_counter.at(*ns_pair.begin()); }
  return priority;
}

// Same as above, returns 0 (includes rest to remove unused variable warning)
float calc_priority_empty(
    const VW::reductions::automl::exclusion_config& config, const std::map<VW::namespace_index, uint64_t>& ns_counter)
{
  _UNUSED(config);
  _UNUSED(ns_counter);
  return 0.f;
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::automl_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();

  uint64_t global_lease;
  uint64_t max_live_configs;
  std::string cm_type;
  std::string priority_type;
  int32_t priority_challengers;
  bool verbose_metrics = false;
  std::string interaction_type;
  std::string oracle_type;
  float automl_significance_level;
  float automl_estimator_decay;
  bool reversed_learning_order = false;
  bool lb_trick = false;
  bool fixed_significance_level;
  std::string predict_only_model_file;

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
               .default_value("none")
               .one_of({"none", "favor_popular_namespaces"})
               .help("Set function to determine next config")
               .experimental())
      .add(make_option("priority_challengers", priority_challengers)
               .keep()
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
               .default_value("one_diff")
               .one_of({"one_diff", "rand", "champdupe"})
               .help("Set oracle to generate configs")
               .experimental())
      .add(make_option("debug_reversed_learn", reversed_learning_order)
               .default_value(false)
               .help("Debug: learn each config in reversed order (last to first).")
               .experimental())
      .add(make_option("lb_trick", lb_trick)
               .default_value(false)
               .help("Use 1-lower_bound as upper_bound for estimator")
               .experimental())
      .add(make_option("aml_predict_only_model", predict_only_model_file)
               .default_value("")
               .help("transform input automl model into predict only automl model")
               .experimental())
      .add(make_option("automl_significance_level", automl_significance_level)
               .keep()
               .default_value(DEFAULT_ALPHA)
               .help("Set significance level for champion change")
               .experimental())
      .add(make_option("automl_estimator_decay", automl_estimator_decay)
               .keep()
               .default_value(CRESSEREAD_DEFAULT_TAU)
               .help("Time constant for count decay")
               .experimental())
      .add(make_option("fixed_significance_level", fixed_significance_level)
               .keep()
               .help("Use fixed significance level as opposed to scaling by model count (bonferroni correction)")
               .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  VW::reductions::automl::priority_func* calc_priority;

  if (priority_type == "none") { calc_priority = &calc_priority_empty; }
  else if (priority_type == "favor_popular_namespaces")
  {
    calc_priority = &calc_priority_favor_popular_namespaces;
  }
  else
  {
    THROW("Invalid priority function provided");
  }

  if (priority_challengers < 0) { priority_challengers = (static_cast<int>(max_live_configs) - 1) / 2; }

  if (!fixed_significance_level) { automl_significance_level /= max_live_configs; }

  bool predict_only_model = options.was_supplied("aml_predict_only_model");

  // Note that all.wpp will not be set correctly until after setup
  auto cm = VW::make_unique<VW::reductions::automl::interaction_config_manager>(global_lease, max_live_configs,
      all.get_random_state(), static_cast<uint64_t>(priority_challengers), interaction_type, oracle_type,
      all.weights.dense_weights, calc_priority, automl_significance_level, automl_estimator_decay, &all.logger, all.wpp,
      lb_trick);
  auto data = VW::make_unique<VW::reductions::automl::automl<VW::reductions::automl::interaction_config_manager>>(
      std::move(cm), &all.logger, predict_only_model);
  data->debug_reverse_learning_order = reversed_learning_order;
  data->cm->per_live_model_state_double = std::vector<double>(max_live_configs * 2, 0.f);
  data->cm->per_live_model_state_uint64 = std::vector<uint64_t>(max_live_configs * 2, 0.f);

  if (max_live_configs > VW::reductions::automl::MAX_CONFIGS)
  {
    THROW("Maximum number of configs is "
        << VW::reductions::automl::MAX_CONFIGS << " and " << max_live_configs
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

  VW::reductions::fail_if_enabled(all,
      {"ccb_explore_adf", "audit_regressor", "baseline", "cb_explore_adf_rnd", "cb_to_cb_adf", "cbify", "replay_c",
          "replay_b", "replay_m", "memory_tree", "new_mf", "nn", "stage_poly"});

  // only this has been tested
  if (base_learner->is_multiline())
  {
    auto ppw = max_live_configs;
    auto* persist_ptr = verbose_metrics ? persist<VW::reductions::automl::interaction_config_manager, true>
                                        : persist<VW::reductions::automl::interaction_config_manager, false>;
    data->adf_learner = as_multiline(base_learner->get_learner_by_name_prefix("cb_adf"));
    GD::gd& gd = *static_cast<GD::gd*>(
        base_learner->get_learner_by_name_prefix("gd")->get_internal_type_erased_data_pointer_test_use_only());
    auto& adf_data =
        *static_cast<CB_ADF::cb_adf*>(data->adf_learner->get_internal_type_erased_data_pointer_test_use_only());
    data->cm->_gd_normalized = &(gd.per_model_states[0].normalized_sum_norm_x);
    data->cm->_gd_total_weight = &(gd.per_model_states[0].total_weight);
    data->cm->_cb_adf_event_sum = &(adf_data._gen_cs.event_sum);
    data->cm->_cb_adf_action_sum = &(adf_data._gen_cs.action_sum);

    auto* l = make_reduction_learner(std::move(data), as_multiline(base_learner),
        learn_automl<VW::reductions::automl::interaction_config_manager, true>,
        predict_automl<VW::reductions::automl::interaction_config_manager, true>,
        stack_builder.get_setupfn_name(automl_setup))
                  .set_params_per_weight(ppw)  // refactor pm
                  .set_output_prediction_type(VW::prediction_type_t::action_scores)
                  .set_input_label_type(VW::label_type_t::cb)
                  .set_input_prediction_type(VW::prediction_type_t::action_scores)
                  .set_output_label_type(VW::label_type_t::cb)
                  .set_finish_example(::finish_example<VW::reductions::automl::interaction_config_manager>)
                  .set_save_load(save_load_aml<VW::reductions::automl::interaction_config_manager>)
                  .set_persist_metrics(persist_ptr)
                  .set_output_prediction_type(base_learner->get_output_prediction_type())
                  .set_learn_returns_prediction(true)
                  .build();
    return make_base(*l);
  }
  else
  {
    // not implemented yet
    THROW("fatal: automl not supported for single line learners yet");
  }
}
