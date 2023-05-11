// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/interaction_ground.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/constant.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/loss_functions.h"
#include "vw/core/multi_model_utils.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/prediction_type.h"
#include "vw/core/print_utils.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label.h"
#include "vw/core/vw.h"

using namespace VW::LEARNER;
using namespace VW::config;
using namespace VW::reductions;

namespace VW
{
namespace reductions
{
namespace igl
{
igl_data::igl_data(bool predict_only_model) : _predict_only_model(predict_only_model) {}
}  // namespace igl

namespace model_utils
{
size_t write_model_field(
    io_buf& io, const VW::reductions::igl::igl_data& igl, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, *igl.pi_ftrl.get(), upstream_name + ".pi", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::reductions::igl::igl_data& igl)
{
  size_t bytes = 0;
  bytes += read_model_field(io, *igl.pi_ftrl.get());
  return bytes;
}
}  // namespace model_utils
}  // namespace reductions
}  // namespace VW

namespace
{
// ik stands for inverse kinematics, ik model is taking in context and feedback
// to predict distribution over actions
class ik_stack_builder : public VW::default_reduction_stack_setup
{
public:
  ik_stack_builder(std::shared_ptr<learner> ftrl_coin) : _ik_base(ftrl_coin)
  {
    std::set<std::string> keep = {"scorer", "count_label"};

    for (size_t i = _reduction_stack.size(); i > 0; i--)
    {
      if (keep.count(std::get<0>(_reduction_stack.at(i - 1))) == 0)
      {
        _reduction_stack.erase(_reduction_stack.begin() + i - 1);
      }
    }
  }

  std::shared_ptr<VW::LEARNER::learner> setup_base_learner(size_t increment = 1) override
  {
    if (_reduction_stack.size() == 0) { return _ik_base; }

    return VW::default_reduction_stack_setup::setup_base_learner(increment);
  }

private:
  std::shared_ptr<VW::LEARNER::learner> _ik_base;
};

std::vector<std::vector<VW::namespace_index>> get_ik_interactions(
    const std::vector<std::vector<VW::namespace_index>>& interactions, const VW::example& observation_ex)
{
  std::vector<std::vector<VW::namespace_index>> new_interactions;
  for (const auto& interaction : interactions)
  {
    for (auto obs_ns : observation_ex.indices)
    {
      if (obs_ns == VW::details::DEFAULT_NAMESPACE) { obs_ns = VW::details::IGL_FEEDBACK_NAMESPACE; }

      new_interactions.push_back(interaction);
      new_interactions.back().push_back(obs_ns);
    }
  }

  return new_interactions;
}

void add_obs_features_to_ik_ex(VW::example& ik_ex, const VW::example& obs_ex)
{
  for (auto obs_ns : obs_ex.indices)
  {
    ik_ex.indices.push_back(obs_ns);

    for (size_t i = 0; i < obs_ex.feature_space[obs_ns].indices.size(); i++)
    {
      auto feature_hash = obs_ex.feature_space[obs_ns].indices[i];
      auto feature_val = obs_ex.feature_space[obs_ns].values[i];

      if (obs_ns == VW::details::DEFAULT_NAMESPACE) { obs_ns = VW::details::IGL_FEEDBACK_NAMESPACE; }

      ik_ex.feature_space[obs_ns].indices.push_back(feature_hash);
      ik_ex.feature_space[obs_ns].values.push_back(feature_val);
    }
  }
}

void learn(VW::reductions::igl::igl_data& igl, learner& base, VW::multi_ex& ec_seq)
{
  float p_unlabeled_prior = 0.5f;

  std::swap(igl.ik_ftrl->all->loss_config.loss, igl.ik_all->loss_config.loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float ik_pred = 0.f;
  size_t chosen_action_idx = 0;

  const auto it = std::find_if(ec_seq.begin(), ec_seq.end(),
      [](const VW::example* item) { return !item->l.cb_with_observations.event.costs.empty(); });

  if (it != ec_seq.end()) { chosen_action_idx = std::distance(ec_seq.begin(), it); }

  VW::example* observation_ex = nullptr;

  if (ec_seq.back()->l.cb_with_observations.is_observation)
  {
    observation_ex = ec_seq.back();
    ec_seq.pop_back();
  }

  auto ik_interactions = get_ik_interactions(igl.interactions, *observation_ex);

  for (size_t i = 0; i < ec_seq.size(); i++)
  {
    auto action_ex = ec_seq[i];
    VW::empty_example(*igl.ik_ftrl->all, igl.ik_ex);

    // TODO: Do we need constant feature here? If so, VW::add_constant_feature
    VW::details::append_example_namespaces_from_example(igl.ik_ex, *action_ex);

    add_obs_features_to_ik_ex(igl.ik_ex, *observation_ex);
    // 1. set up ik ex
    igl.ik_ex.l.simple.label = i == chosen_action_idx ? 1.f : -1.f;

    auto action_score_iter = std::find_if(ec_seq[0]->pred.a_s.begin(), ec_seq[0]->pred.a_s.end(),
        [i](const VW::action_score& element) { return element.action == i; });

    float pa = action_score_iter->score;

    if (i == chosen_action_idx) { igl.ik_ex.weight = 1 / 4.f / pa; }
    else { igl.ik_ex.weight = 3 / 4.f / (1 - pa); }

    igl.ik_ex.interactions = &ik_interactions;
    // TODO(low pri): not reuse igl.extent_interactions, need to add in feedback
    igl.ik_ex.extent_interactions = igl.extent_interactions;

    // 2. ik learn
    igl.ik_learner->learn(igl.ik_ex, 0);

    if (i == chosen_action_idx) { ik_pred = igl.ik_ex.pred.scalar; }

    action_ex->l.cb = action_ex->l.cb_with_observations.event;
    action_ex->l.cb_with_observations.event.reset_to_default();
  }

  std::swap(igl.ik_ftrl->all->loss_config.loss, igl.ik_all->loss_config.loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float predicted_cost = 0.f;
  // 4. update multi line ex label
  if (ik_pred * 2 > 1.f)
  {
    int is_definitely_bad = static_cast<int>(observation_ex->l.cb_with_observations.is_definitely_bad);
    predicted_cost = -1.f + is_definitely_bad * (1.f + 1.f / p_unlabeled_prior);
  }

  // 5. Train pi policy
  ec_seq[chosen_action_idx]->l.cb.costs[0].cost = predicted_cost;

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  // TODO: switch the label parser type to CB for text format

  // reset the first_observed label for cb_explore_adf reduction
  (*igl.ik_ftrl).all->sd->first_observed_label = FLT_MAX;
  base.learn(ec_seq, 1);
  igl.known_cost = VW::get_observed_cost_or_default_cb_adf(ec_seq);

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  for (auto& ex : ec_seq)
  {
    ex->l.cb_with_observations.event = ex->l.cb;
    ex->l.cb.reset_to_default();
  }

  ec_seq.push_back(observation_ex);
}

void predict(VW::reductions::igl::igl_data& igl, learner& base, VW::multi_ex& ec_seq)
{
  VW::example* observation_ex = nullptr;

  if (ec_seq.size() > 0 && ec_seq.back()->l.cb_with_observations.is_observation)
  {
    observation_ex = ec_seq.back();
    ec_seq.pop_back();
  }

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  for (auto& ex : ec_seq)
  {
    ex->l.cb = ex->l.cb_with_observations.event;
    ex->l.cb_with_observations.event.reset_to_default();
  }

  base.predict(ec_seq, 1);
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  for (auto& ex : ec_seq)
  {
    ex->l.cb_with_observations.event = ex->l.cb;
    ex->l.cb.reset_to_default();
  }

  if (observation_ex != nullptr) { ec_seq.push_back(observation_ex); }
}

void save_load_igl(VW::reductions::igl::igl_data& igl, VW::io_buf& io, bool read, bool text)
{
  if (io.num_files() == 0) { return; }
  if (read) { VW::reductions::model_utils::read_model_field(io, igl); }
  else if (!igl._predict_only_model) { VW::reductions::model_utils::write_model_field(io, igl, "igl", text); }
}

void print_update_igl(VW::workspace& all, VW::shared_data& /*sd*/, const VW::reductions::igl::igl_data& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /*logger*/)
{
  if (ec_seq.empty()) { return; }
  const auto& ec = **(ec_seq.begin());
  if (VW::example_is_newline(ec) && !VW::ec_is_example_header_cb_with_observations(ec)) { return; }

  bool labeled_example = true;
  if (data.known_cost.probability <= 0) { labeled_example = false; }

  VW::details::print_update_cb(all, !labeled_example, ec, &ec_seq, true, &data.known_cost);
}

void pre_save_load_igl(VW::workspace& all, igl::igl_data& data)
{
  options_i& options = *all.options;
  if (!data._predict_only_model) { return; }

  auto cb_adf_learner = all.l->get_learner_by_name_prefix("cb_adf")->shared_from_this();
  auto cb_adf_data = static_cast<cb_adf*>(cb_adf_learner->get_internal_type_erased_data_pointer_test_use_only());

  std::swap(cb_adf_data->get_gen_cs_mtr().per_model_state[0], cb_adf_data->get_gen_cs_mtr().per_model_state[1]);

  // Adjust pi model weights to new single-model space
  VW::reductions::multi_model::reduce_innermost_model_weights(
      all.weights.dense_weights, data._cb_model_offset, all.reduction_state.total_feature_width, data._feature_width);

  for (auto& group : options.get_all_option_group_definitions())
  {
    if (group.m_name == "[Reduction] Interaction Grounded Learning Options")
    {
      for (auto& opt : group.m_options) { opt->m_keep = false; }
    }
  }

  all.initial_weights_config.num_bits =
      all.initial_weights_config.num_bits - static_cast<uint32_t>(std::log2(all.reduction_state.total_feature_width));
  options.get_typed_option<uint32_t>("bit_precision").value(all.initial_weights_config.num_bits);
}

void output_igl_prediction(
    VW::workspace& all, const igl::igl_data& /* data */, const VW::multi_ex& ec_seq, VW::io::logger& /* unused */)
{
  if (!ec_seq.empty())
  {
    // Print predictions
    for (auto& sink : all.output_runtime.final_prediction_sink)
    {
      VW::details::print_action_score(sink.get(), ec_seq[VW::details::SHARED_EX_INDEX]->pred.a_s,
          ec_seq[VW::details::SHARED_EX_INDEX]->tag, all.logger);
    }
    VW::details::global_print_newline(all.output_runtime.final_prediction_sink, all.logger);
  }
}

void update_stats_igl(const VW::workspace& /* all */, VW::shared_data& sd, const VW::reductions::igl::igl_data& data,
    const VW::multi_ex& ec_seq, VW::io::logger& /*logger*/)
{
  if (ec_seq.size() <= 0) { return; }

  size_t num_features = 0;

  float loss = 0.;

  auto& ec = *ec_seq[0];
  const auto& preds = ec.pred.a_s;

  for (const auto& example : ec_seq)
  {
    if (VW::ec_is_example_header_cb_with_observations(*example))
    {
      num_features += (ec_seq.size() - 1) *
          (example->get_num_features() - example->feature_space[VW::details::CONSTANT_NAMESPACE].size());
    }
    else { num_features += example->get_num_features(); }
  }

  // TODO: _metrics?

  bool labeled_example = true;
  if (data.known_cost.probability > 0)
  {
    for (uint32_t i = 0; i < preds.size(); i++)
    {
      float l = VW::get_cost_estimate(data.known_cost, preds[i].action);
      loss += l * preds[i].score * ec_seq[ec_seq.size() - preds.size() + i]->weight;
    }
  }
  else { labeled_example = false; }

  bool holdout_example = labeled_example;
  for (size_t i = 0; i < ec_seq.size(); i++) { holdout_example &= ec_seq[i]->test_only; }

  sd.update(holdout_example, labeled_example, loss, ec.weight, num_features);
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::interaction_ground_setup(VW::setup_base_i& stack_builder)
{
  options_i& pi_options = *stack_builder.get_options();
  VW::workspace* all = stack_builder.get_all_pointer();
  bool igl_option = false;
  option_group_definition new_options("[Reduction] Interaction Grounded Learning");
  new_options.add(make_option("experimental_igl", igl_option)
                      .keep()
                      .necessary()
                      .help("Do Interaction Grounding with multiline action dependent features")
                      .experimental());

  if (!pi_options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // number of weight vectors needed, one for ik model(offset 0) and one for cb model(offset 1)
  size_t feature_width = 2;
  bool predict_only_model = pi_options.was_supplied("predict_only_model");

  auto ld = VW::make_unique<VW::reductions::igl::igl_data>(predict_only_model);

  if (!pi_options.was_supplied("cb_explore_adf")) { pi_options.insert("cb_explore_adf", ""); }

  if (!pi_options.was_supplied("coin")) { THROW("--experimental_igl needs to use --coin"); }

  auto pi_learner = require_multiline(stack_builder.setup_base_learner(feature_width));

  // 1. fetch already allocated coin reduction
  auto ftrl_coin = pi_learner->get_learner_by_name_prefix("ftrl-Coin")->shared_from_this();

  // 2. prepare args for ik stack
  std::vector<std::string> ik_args = {"--quiet", "--link=logistic", "--loss_function=logistic", "--coin"};
  std::unique_ptr<options_i, options_deleter_type> ik_options(
      new config::options_cli(ik_args), [](VW::config::options_i* ptr) { delete ptr; });

  assert(ik_options->was_supplied("cb_explore_adf") == false || ik_options->was_supplied("cb_adf") == false);
  assert(ik_options->was_supplied("loss_function") == true);

  ld->ik_ftrl = static_cast<ftrl*>(ftrl_coin->get_internal_type_erased_data_pointer_test_use_only());
  ld->pi_ftrl = VW::make_unique<ftrl>();
  *ld->pi_ftrl.get() = *ld->ik_ftrl;

  ld->ik_all = VW::initialize_experimental(
      VW::make_unique<VW::config::options_cli>(ik_args), nullptr, nullptr, nullptr, &all->logger);

  std::unique_ptr<ik_stack_builder> ik_builder = VW::make_unique<ik_stack_builder>(ftrl_coin);

  ik_builder->delayed_state_attach(*ld->ik_ftrl->all, *ik_options);
  ld->ik_learner = require_singleline(ik_builder->setup_base_learner());

  ld->interactions = all->feature_tweaks_config.interactions;
  ld->extent_interactions = &all->feature_tweaks_config.extent_interactions;

  VW::prediction_type_t pred_type;

  if (pi_learner->get_output_prediction_type() == prediction_type_t::ACTION_SCORES)
  {
    pred_type = prediction_type_t::ACTION_SCORES;
  }
  else if (pi_learner->get_output_prediction_type() == prediction_type_t::ACTION_PROBS)
  {
    pred_type = prediction_type_t::ACTION_PROBS;
  }
  else { THROW("--experimental_igl not implemented for this type of prediction"); }

  auto l = make_reduction_learner(
      std::move(ld), pi_learner, learn, predict, stack_builder.get_setupfn_name(interaction_ground_setup))
               .set_feature_width(feature_width)
               .set_input_label_type(label_type_t::CB_WITH_OBSERVATIONS)
               .set_output_label_type(label_type_t::CB)
               .set_input_prediction_type(pred_type)
               .set_output_prediction_type(pred_type)
               .set_update_stats(update_stats_igl)
               .set_print_update(print_update_igl)
               .set_save_load(save_load_igl)
               .set_output_example_prediction(output_igl_prediction)
               .set_pre_save_load(pre_save_load_igl)
               .build();

  // TODO: assert ftrl is the base, fail otherwise
  // VW::reductions::util::fail_if_enabled
  return l;
}
