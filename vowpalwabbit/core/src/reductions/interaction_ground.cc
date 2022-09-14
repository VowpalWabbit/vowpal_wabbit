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

class interaction_ground
{
public:
  std::shared_ptr<learner> ik_learner = nullptr;
  VW::example ik_ex;

  std::vector<std::vector<namespace_index>> ik_interactions;
  std::vector<std::vector<extent_term>>* extent_interactions;

  std::unique_ptr<VW::workspace> ik_all;
  ftrl* ik_ftrl; // automatically save resume
  std::unique_ptr<ftrl> pi_ftrl; // TODO: add save resume
  ~interaction_ground() {
  }

  // TODO: rule of 5
};

void learn(interaction_ground& igl, learner& base, VW::multi_ex& ec_seq)
{
  float p_unlabeled_prior = 0.5f;

  std::swap(igl.ik_ftrl->all->loss, igl.ik_all->loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float ik_pred = 0.f;
  int chosen_action_idx = 0;

  const auto it = std::find_if(ec_seq.begin(), ec_seq.end(), [](VW::example* item) { return !item->l.cb.costs.empty(); });
  if (it != ec_seq.end())
  {
    chosen_action_idx = std::distance(ec_seq.begin(), it);
  }

  auto feedback_ex = ec_seq.back(); // TODO: refactor. Now assume last example is feedback example
  ec_seq.pop_back();

  auto ns_iter = feedback_ex->indices.begin();
  uint64_t feature_hash = *feedback_ex->feature_space.at(*ns_iter).indices.data();

  for (auto& ex_action : ec_seq) {
    VW::empty_example(*igl.ik_all, igl.ik_ex);
    // TODO: Do we need constant feature here? If so, VW::add_constant_feature
    VW::details::append_example_namespaces_from_example(igl.ik_ex, *ex_action);
    igl.ik_ex.indices.push_back(VW::details::IGL_FEEDBACK_NAMESPACE);
    igl.ik_ex.feature_space[VW::details::IGL_FEEDBACK_NAMESPACE].push_back(1, feature_hash); // TODO: is the feature value always 1?
    igl.ik_ex.num_features++;

    // 1. set up ik ex
    float ik_label = -1.f;
    if (!ex_action->l.cb.costs.empty()) {
      ik_label = 1.f;
    }
    igl.ik_ex.l.simple.label = ik_label;

    // TODO: update the importance for each example
    float importance = 0.6f;
    igl.ik_ex.weight = importance;

    igl.ik_ex.interactions = &igl.ik_interactions;
    igl.ik_ex.extent_interactions = igl.extent_interactions; // TODO(low pri): not reuse igl.extent_interactions, need to add in feedback

    // 2. ik learn
    igl.ik_learner->learn(igl.ik_ex, 0);

    if (!ex_action->l.cb.costs.empty()) {
      ik_pred = igl.ik_ex.pred.scalar;
    }
    // TODO: shared data print needs to be fixed
  }

  std::swap(igl.ik_ftrl->all->loss, igl.ik_all->loss);
  std::swap(igl.ik_ftrl->all->sd, igl.ik_all->sd);

  float fake_cost = 0.f;
  // 4. update multi line ex label
  if (ik_pred * 2 > 1) {
    // TODO: get Definitely Bad label from feedback example
    bool is_neg = 1;
    fake_cost = -p_unlabeled_prior + is_neg * (1 + p_unlabeled_prior); // TODO: update to latest version
  }

  // 5. Train pi policy
  ec_seq[chosen_action_idx]->l.cb.costs[0].cost = fake_cost;

  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
  base.learn(ec_seq, 1);
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
  ec_seq.push_back(feedback_ex);
}

void predict(interaction_ground& igl, learner& base, VW::multi_ex& ec_seq)
{
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
  base.predict(ec_seq, 1);
  std::swap(*igl.pi_ftrl.get(), *igl.ik_ftrl);
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

  // Ensure cb_explore_adf so we are reducing to something useful.
  // Question: Can we support both cb_adf and cb_explore_adf?
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

  std::unique_ptr<ik_stack_builder> ik_builder = VW::make_unique<ik_stack_builder>(ftrl_coin);
  ik_builder->delayed_state_attach(*ld->ik_all, *ik_options);
  ld->ik_learner = require_singleline(ik_builder->setup_base_learner());

  ld->ik_ftrl = static_cast<ftrl*>(ftrl_coin->get_internal_type_erased_data_pointer_test_use_only());
  ld->pi_ftrl = VW::make_unique<ftrl>();
  *ld->pi_ftrl.get() = *ld->ik_ftrl;

  for (auto& interaction : all->interactions) {
    interaction.push_back(VW::details::IGL_FEEDBACK_NAMESPACE);
    ld->ik_interactions.push_back(interaction);
    interaction.pop_back();
  }

  ld->extent_interactions = &all->extent_interactions;

  auto l = make_reduction_learner(
      std::move(ld), pi_learner, predict, stack_builder.get_setupfn_name(interaction_ground_setup))
               .set_feature_width(feature_width)
               .set_input_label_type(label_type_t::CB)
               .set_output_label_type(label_type_t::CB)
               .set_output_prediction_type(prediction_type_t::ACTION_SCORES)
               .set_input_prediction_type(prediction_type_t::ACTION_SCORES)
               .build();

  // TODO: assert ftrl is the base, fail otherwise
  // VW::reductions::util::fail_if_enabled
  return l;
}
