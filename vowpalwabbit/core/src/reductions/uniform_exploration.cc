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

template <bool is_learn, bool is_threshold>
void predict_or_learn(uniform_exploration_data& data, VW::LEARNER::multi_learner& base, VW::multi_ex& ec_seq)
{
  if VW_STD17_CONSTEXPR (is_learn) { base.learn(ec_seq); }
  else { base.predict(ec_seq); }

  auto& probs = ec_seq[0]->pred.a_s;
  const auto size = probs.size();
  if VW_STD17_CONSTEXPR (is_threshold)
  {
    const auto minimum_probability = data._uniform_epsilon / size;

    size_t n_changed = 0;
    float p_unchanged = 0.f;

    for (auto& prob : probs)
    {
      if (prob.score < minimum_probability) { n_changed += 1; }
      else { p_unchanged += prob.score; }
    }

    for (auto& prob : probs)
    {
      if (prob.score < minimum_probability) { prob.score = minimum_probability; }
      else { prob.score *= (1 - n_changed * minimum_probability) / p_unchanged; }
    }
  }
  else
  {
    const auto scale = (1.f - data._uniform_epsilon);
    for (auto& prob : probs)
    {
      prob.score *= scale;
      prob.score += data._uniform_epsilon / size;
    }
  }
}
}  // namespace

VW::LEARNER::base_learner* VW::reductions::uniform_exploration_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();

  float uniform_epsilon{};
  std::string kind;
  VW::config::option_group_definition new_options("[Reduction] Uniform exploration");
  new_options.add(VW::config::make_option("uniform_epsilon", uniform_epsilon).keep().necessary().help(""));
  // Threshold - sets any actions below the minimum probability to the minimum probability and renormalizes the
  // remaining probabilities Mixture - enforces minimum probability by mixing the action distribution with the uniform
  // distribution (p <- (1-eps)*p + eps*uniform)
  //           In other words, with prob 1-eps follow the original action distribution (pmf), with prob eps you follow
  //           the uniform distribution
  new_options.add(VW::config::make_option("uniform_epsilon_kind", kind)
                      .keep()
                      .default_value("threshold")
                      .help("")
                      .one_of({"threshold", "mixture"}));
  if (!options.add_parse_and_check_necessary(new_options)) { return nullptr; }

  if (uniform_epsilon < 0.f || uniform_epsilon > 1.f) { THROW("uniform_epsilon must be in the range [0, 1]"); }

  auto* multi_base = VW::LEARNER::as_multiline(stack_builder.setup_base_learner());
  auto data = VW::make_unique<uniform_exploration_data>(uniform_epsilon);

  using predict_or_learn_ptr =
      void (*)(::uniform_exploration_data & data, VW::LEARNER::multi_learner & base, VW::multi_ex & ec_seq);
  predict_or_learn_ptr pred_ptr;
  predict_or_learn_ptr learn_ptr;

  if (kind == "threshold")
  {
    pred_ptr = predict_or_learn<false, true>;
    learn_ptr = predict_or_learn<true, true>;
  }
  else if (kind == "mixture")
  {
    pred_ptr = predict_or_learn<false, false>;
    learn_ptr = predict_or_learn<true, false>;
  }
  else { THROW("uniform_epsilon_kind must be one of threshold or mixture"); }

  // Label type is inherited from base
  auto* learner = VW::LEARNER::make_reduction_learner(
      std::move(data), multi_base, learn_ptr, pred_ptr, stack_builder.get_setupfn_name(uniform_exploration_setup))
                      .set_input_prediction_type(VW::prediction_type_t::action_probs)
                      .set_output_prediction_type(VW::prediction_type_t::action_probs)
                      .build();

  return VW::LEARNER::make_base(*learner);
}
