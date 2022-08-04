// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/uniform_exploration.h"

#include "vw/config/options.h"
#include "vw/core/example.h"
#include "vw/core/label_type.h"
#include "vw/core/learner.h"
#include "vw/core/setup_base.h"

namespace
{
struct uniform_exploration_data
{
  uniform_exploration_data(float uniform_epsilon) : _uniform_epsilon(uniform_epsilon) {}
  float _uniform_epsilon;
};

template <bool is_learn>
void predict_or_learn(uniform_exploration_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq)
{
  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec_seq); }
  else
  {
    base.predict(ec_seq);
  }

  auto& probs = ec_seq[0]->pred.a_s;
  const auto size = probs.size();
  const auto scale = (1.f - data._uniform_epsilon);
  for (auto& prob : probs)
  {
    prob.score *= scale;
    prob.score += data._uniform_epsilon / size;
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::uniform_exploration_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();

  float uniform_epsilon = 0.05f;
  VW::config::option_group_definition new_options("[Reduction] Uniform exploration");
  new_options.add(VW::config::make_option("uniform_epsilon", uniform_epsilon).keep().necessary().help(""));
  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  auto* multi_base = VW::LEARNER::as_multiline(stack_builder.setup_base_learner());
  auto data = VW::make_unique<uniform_exploration_data>(uniform_epsilon);

  // Label type is inherited from base
  auto* learner = VW::LEARNER::make_reduction_learner(std::move(data), multi_base, predict_or_learn<true>,
      predict_or_learn<false>, stack_builder.get_setupfn_name(uniform_exploration_setup))
                      .set_input_prediction_type(VW::prediction_type_t::action_probs)
                      .set_output_prediction_type(VW::prediction_type_t::action_probs)
                      .build();

  return VW::LEARNER::make_base(*learner);
}
