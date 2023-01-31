// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/interaction_ground.h"

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/core/label_dictionary.h"
#include "vw/core/label_parser.h"
#include "vw/core/prediction_type.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/setup_base.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

using namespace VW::LEARNER;
using namespace VW::config;

namespace
{
class interaction_ground
{
public:
  // the accumulated importance weighted reward of a policy which optimizes the given value
  double total_importance_weighted_reward = 0.;
  double total_uniform_reward = 0.;
  // the accumulated importance weighted loss of the policy which optimizes the negative of the given value
  double total_importance_weighted_cost = 0.;
  double total_uniform_cost = 0.;
};

void negate_cost(VW::multi_ex& ec_seq)
{
  for (auto* example_ptr : ec_seq)
  {
    for (auto& label : example_ptr->l.cb.costs)
    {
      if (label.has_observed_cost()) { label.cost = -label.cost; }
    }
  }
}

void learn(interaction_ground& ig, learner& base, VW::multi_ex& ec_seq)
{
  // find reward of sequence
  VW::cb_class label = VW::get_observed_cost_or_default_cb_adf(ec_seq);
  ig.total_uniform_cost += label.cost / label.probability / ec_seq.size();  //=p(uniform) * IPS estimate
  ig.total_uniform_reward += -label.cost / label.probability / ec_seq.size();

  // find prediction & update for cost
  base.predict(ec_seq);
  ig.total_importance_weighted_cost += get_cost_estimate(label, ec_seq[0]->pred.a_s[0].action);
  base.learn(ec_seq);

  // find prediction & update for reward
  label.cost = -label.cost;
  base.predict(ec_seq, 1);
  ig.total_importance_weighted_reward += get_cost_estimate(label, ec_seq[0]->pred.a_s[0].action);

  // change cost to reward
  negate_cost(ec_seq);
  base.learn(ec_seq, 1);
  negate_cost(ec_seq);
}

void predict(interaction_ground& ig, learner& base, VW::multi_ex& ec_seq)
{
  // figure out which is better by our current estimate.
  if (ig.total_uniform_cost - ig.total_importance_weighted_cost >
      ig.total_uniform_reward - ig.total_importance_weighted_reward)
  {
    base.predict(ec_seq);
  }
  else { base.predict(ec_seq, 1); }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::interaction_ground_setup(VW::setup_base_i& stack_builder)
{
  options_i& options = *stack_builder.get_options();
  bool igl_option = false;

  option_group_definition new_options("[Reduction] Interaction Grounded Learning");
  new_options.add(make_option("experimental_igl", igl_option)
                      .keep()
                      .necessary()
                      .help("Do Interaction Grounding with multiline action dependent features")
                      .experimental());

  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  // number of weight vectors needed
  size_t problem_multiplier = 2;  // One for reward and one for loss
  auto ld = VW::make_unique<interaction_ground>();

  // Ensure cb_adf so we are reducing to something useful.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  auto base = require_multiline(stack_builder.setup_base_learner());
  auto l = make_reduction_learner(
      std::move(ld), base, learn, predict, stack_builder.get_setupfn_name(interaction_ground_setup))
               .set_params_per_weight(problem_multiplier)
               .set_input_label_type(label_type_t::CB)
               .set_output_label_type(label_type_t::CB)
               .set_output_prediction_type(prediction_type_t::ACTION_SCORES)
               .set_input_prediction_type(prediction_type_t::ACTION_SCORES)
               .build();

  return l;
}
