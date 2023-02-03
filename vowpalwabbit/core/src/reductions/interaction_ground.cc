// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/interaction_ground.h"
#include "vw/config/options_cli.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/prediction_type.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/core/loss_functions.h"
#include "vw/core/simple_label.h"
#include "vw/core/constant.h"
#include "vw/core/parse_primitives.h"

using namespace VW::LEARNER;
using namespace VW::config;

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

    for (int i = _reduction_stack.size() - 1; i >= 0; i--) {
      if (keep.count(std::get<0>(_reduction_stack.at(i))) == 0) {
        _reduction_stack.erase(_reduction_stack.begin() + i);
      }
    }
  }

  std::shared_ptr<learner> setup_base_learner() override {
    if (_reduction_stack.size() == 0) {
      return _ik_base;
    }

    return VW::default_reduction_stack_setup::setup_base_learner();
  }

private:
  std::shared_ptr<learner> _ik_base;
};

std::vector<std::vector<VW::namespace_index>> get_ik_interactions(std::vector<std::vector<VW::namespace_index>> interactions, VW::example* observation_ex) {
  std::vector<std::vector<VW::namespace_index>> new_interactions;
  for (auto& interaction : interactions) {
    for (auto& obs_ns : observation_ex->indices) {
      if (obs_ns == VW::details::DEFAULT_NAMESPACE) {
        obs_ns = VW::details::IGL_FEEDBACK_NAMESPACE;
      }

      new_interactions.push_back(interaction);
      new_interactions.back().push_back(obs_ns);
    }
  }

  return new_interactions;
}

void add_obs_features_to_ik_ex(VW::example* ik_ex, VW::example* obs_ex) {
  for (auto& obs_ns : obs_ex->indices) {
    ik_ex->indices.push_back(obs_ns);

    for (size_t i = 0; i < obs_ex->feature_space[obs_ns].indices.size(); i++) {
      auto feature_hash = obs_ex->feature_space[obs_ns].indices[i];
      auto feature_val = obs_ex->feature_space[obs_ns].values[i];

      if (obs_ns == VW::details::DEFAULT_NAMESPACE) {
        obs_ns = VW::details::IGL_FEEDBACK_NAMESPACE;
      }

      ik_ex->feature_space[obs_ns].indices.push_back(feature_hash);
      ik_ex->feature_space[obs_ns].values.push_back(feature_val);

      // ik_ex->num_features++;
    }
  }
}

class interaction_ground
{
public:
  std::shared_ptr<learner> ik_learner = nullptr;
  VW::example ik_ex;

  std::vector<std::vector<VW::namespace_index>> interactions;
  std::vector<std::vector<VW::extent_term>>* extent_interactions;

  std::unique_ptr<VW::workspace> ik_all;
  ftrl* ik_ftrl; // automatically save resume
  std::unique_ptr<ftrl> pi_ftrl; // TODO: add save resume
  // ~interaction_ground() {
    // auto scorer_stack = ik_learner->get_learner_by_name_prefix("scorer");
    // scorer_stack->_finisher_fd.base = nullptr;
    // delete ik_learner;
  // }

  // TODO: rule of 5
};

void learn(interaction_ground& igl, learner& base, VW::multi_ex& ec_seq)
{
  float p_unlabeled_prior = 0.5f;

  std::swap(igl.ik_ftrl->all->loss, igl.ik_all->loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float ik_pred = 0.f;
  int chosen_action_idx = 0;

  const auto it = std::find_if(ec_seq.begin(), ec_seq.end(), [](VW::example* item) {
    return !item->l.cb_with_observations.event.costs.empty();
  });

  if (it != ec_seq.end()) {
    chosen_action_idx = std::distance(ec_seq.begin(), it);
  }

  VW::example* observation_ex = nullptr;

  if (ec_seq.back()->l.cb_with_observations.is_observation) {
    observation_ex = ec_seq.back();
    ec_seq.pop_back();
  }

  VW::action_scores action_scores = ec_seq[0]->pred.a_s;
  std::vector<std::vector<VW::namespace_index>> ik_interactions = get_ik_interactions(igl.interactions, observation_ex);

  for (size_t i = 0; i < ec_seq.size(); i++) {
    auto action_ex = ec_seq[i];
    VW::empty_example(*igl.ik_all, igl.ik_ex);
    // TODO: Do we need constant feature here? If so, VW::add_constant_feature
    VW::details::append_example_namespaces_from_example(igl.ik_ex, *action_ex);

    add_obs_features_to_ik_ex(&igl.ik_ex, observation_ex);
    // 1. set up ik ex
    float ik_label = -1.f;
    if (!action_ex->l.cb_with_observations.event.costs.empty()) {
      ik_label = 1.f;
    }
    igl.ik_ex.l.simple.label = ik_label;

    if (i == chosen_action_idx) {
      igl.ik_ex.weight = 1/4.f / action_scores[i].score;
    } else {
      igl.ik_ex.weight = 3/4.f / (1 - action_scores[i].score);
    }

    igl.ik_ex.interactions = &ik_interactions;
    igl.ik_ex.extent_interactions = igl.extent_interactions; // TODO(low pri): not reuse igl.extent_interactions, need to add in feedback

    // 2. ik learn
    igl.ik_learner->learn(igl.ik_ex, 0);

    if (!action_ex->l.cb_with_observations.event.costs.empty()) {
      ik_pred = igl.ik_ex.pred.scalar;
    }
    // TODO: shared data print needs to be fixed

    action_ex->l.cb = action_ex->l.cb_with_observations.event;
  }

  std::swap(igl.ik_ftrl->all->loss, igl.ik_all->loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float fake_cost = 0.f;
  // 4. update multi line ex label
  if (ik_pred * 2 > 1) {
    bool is_definitely_bad = observation_ex->l.cb_with_observations.is_definitely_bad;
    fake_cost = -p_unlabeled_prior + is_definitely_bad * (1 + p_unlabeled_prior);
  }

  // 5. Train pi policy
  ec_seq[chosen_action_idx]->l.cb.costs[0].cost = fake_cost;

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  // reset the first_observed label for cb_adf reduction
  (*igl.ik_ftrl).all->sd->first_observed_label = FLT_MAX;
  base.learn(ec_seq, 1);
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
  ec_seq.push_back(observation_ex);
}

void predict(interaction_ground& igl, learner& base, VW::multi_ex& ec_seq)
{
  VW::example* observation_ex = nullptr;

  if (ec_seq.size() > 0) {
    observation_ex = ec_seq.back();
    ec_seq.pop_back();
  }

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
  base.predict(ec_seq, 1);
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);

  if (observation_ex != nullptr) {
    ec_seq.push_back(observation_ex);
  }
}
} // namespace

void copy_ftrl(ftrl* source, ftrl* destination) {
  *destination = *source;
}

std::shared_ptr<VW::LEARNER::learner> VW::reductions::interaction_ground_setup(VW::setup_base_i& stack_builder)
{
  options_i& pi_options = *stack_builder.get_options();
  VW::workspace *all = stack_builder.get_all_pointer();
  bool igl_option = false;
  option_group_definition new_options("[Reduction] Interaction Grounded Learning");
  new_options.add(make_option("experimental_igl", igl_option)
                  .keep()
                  .necessary()
                  .help("Do Interaction Grounding with multiline action dependent features")
                  .experimental());

  if (!pi_options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // number of weight vectors needed
  size_t feature_width = 2;  // One for reward and one for loss
  auto ld = VW::make_unique<interaction_ground>();

  if (!pi_options.was_supplied("cb_explore_adf")) {
    pi_options.insert("cb_explore_adf", "");
  }

  auto pi_learner = require_multiline(stack_builder.setup_base_learner(feature_width));

  // 1. fetch already allocated coin reduction
  auto ftrl_coin = pi_learner->get_learner_by_name_prefix("ftrl-Coin")->shared_from_this();

  // 2. prepare args for ik stack
  std::string ik_args = "--quiet --link=logistic --loss_function=logistic --coin";
  std::unique_ptr<options_i, options_deleter_type> ik_options(
    new config::options_cli(VW::split_command_line(ik_args)),
    [](VW::config::options_i* ptr) { delete ptr; });

  assert(ik_options->was_supplied("cb_explore_adf") == false);
  assert(ik_options->was_supplied("loss_function") == true);

  ld->ik_all = VW::initialize_experimental(VW::make_unique<VW::config::options_cli>(VW::split_command_line(ik_args)));
  all->example_parser->lbl_parser = VW::get_label_parser(label_type_t::CB_WITH_OBSERVATIONS);

  std::unique_ptr<ik_stack_builder> ik_builder = VW::make_unique<ik_stack_builder>(ftrl_coin);
  ik_builder->delayed_state_attach(*ld->ik_all, *ik_options);
  ld->ik_learner = require_singleline(ik_builder->setup_base_learner());

  ld->ik_ftrl = static_cast<ftrl*>(ftrl_coin->get_internal_type_erased_data_pointer_test_use_only());
  ld->pi_ftrl = VW::make_unique<ftrl>();
  *ld->pi_ftrl.get() = *ld->ik_ftrl;

  ld->interactions = all->interactions;
  ld->extent_interactions = &all->extent_interactions;

  auto l = make_reduction_learner(
      std::move(ld), pi_learner, learn, predict, stack_builder.get_setupfn_name(interaction_ground_setup))
               ..set_feature_width(feature_width)
               .set_input_label_type(label_type_t::CB_WITH_OBSERVATIONS)
               .set_output_label_type(label_type_t::CB)
               .set_output_prediction_type(prediction_type_t::ACTION_SCORES)
               .set_input_prediction_type(prediction_type_t::ACTION_SCORES)
               .build();

  // TODO: assert ftrl is the base, fail otherwise
  // VW::reductions::util::fail_if_enabled
  return l;
}
