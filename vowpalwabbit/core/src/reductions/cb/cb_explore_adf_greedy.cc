// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/cb/cb_explore_adf_greedy.h"

#include "vw/config/options.h"
#include "vw/core/gen_cs_example.h"
#include "vw/core/global_data.h"
#include "vw/core/label_parser.h"
#include "vw/core/parser.h"
#include "vw/core/reductions/cb/cb_adf.h"
#include "vw/core/reductions/cb/cb_explore.h"
#include "vw/core/reductions/cb/cb_explore_adf_common.h"
#include "vw/core/setup_base.h"
#include "vw/explore/explore.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
using namespace VW::cb_explore_adf;

namespace
{
class cb_explore_adf_greedy
{
public:
  cb_explore_adf_greedy(float epsilon, bool first_only);
  ~cb_explore_adf_greedy() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::learner& base, VW::multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(VW::LEARNER::learner& base, VW::multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

private:
  float _epsilon;
  bool _first_only;
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::learner& base, VW::multi_ex& examples);
  void update_example_prediction(VW::multi_ex& examples);
};

cb_explore_adf_greedy::cb_explore_adf_greedy(float epsilon, bool first_only)
    : _epsilon(epsilon), _first_only(first_only)
{
}

void cb_explore_adf_greedy::update_example_prediction(VW::multi_ex& examples)
{
  VW::action_scores& preds = examples[0]->pred.a_s;
  uint32_t num_actions = static_cast<uint32_t>(preds.size());

  auto& ep_fts = examples[0]->ex_reduction_features.template get<VW::cb_explore_adf::greedy::reduction_features>();
  float actual_ep = (ep_fts.valid_epsilon_supplied()) ? ep_fts.epsilon : _epsilon;

  size_t tied_actions = fill_tied(preds);

  const float prob = actual_ep / num_actions;
  for (size_t i = 0; i < num_actions; i++) { preds[i].score = prob; }
  if (!_first_only)
  {
    for (size_t i = 0; i < tied_actions; ++i) { preds[i].score += (1.f - actual_ep) / tied_actions; }
  }
  else { preds[0].score += 1.f - actual_ep; }
}

template <bool is_learn>
void cb_explore_adf_greedy::predict_or_learn_impl(VW::LEARNER::learner& base, VW::multi_ex& examples)
{
  // Explore uniform random an epsilon fraction of the time.
  if (is_learn)
  {
    base.learn(examples);
    if (base.learn_returns_prediction) { update_example_prediction(examples); }
  }
  else
  {
    base.predict(examples);
    update_example_prediction(examples);
  }
}
}  // namespace

std::shared_ptr<VW::LEARNER::learner> VW::reductions::cb_explore_adf_greedy_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  bool first_only = false;

  config::option_group_definition new_options("[Reduction] Contextual Bandit Exploration with ADF (greedy)");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon)
               .default_value(0.05f)
               .keep()
               .allow_override()
               .help("Epsilon-greedy exploration"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"));

  // This is a special case "cb_explore_adf" is needed to enable this. BUT it is only enabled when all of the other
  // "cb_explore_adf" types are disabled. This is why we don't check the return value of the
  // add_parse_and_check_necessary call like we do elsewhere.
  options.add_parse_and_check_necessary(new_options);

  // NOTE: epsilon-greedy is the default explore type.
  // This basically runs if none of the other explore strategies are used
  bool use_greedy = !(options.was_supplied("first") || options.was_supplied("bag") || options.was_supplied("cover") ||
      options.was_supplied("regcb") || options.was_supplied("regcbopt") || options.was_supplied("squarecb") ||
      options.was_supplied("rnd") || options.was_supplied("softmax") || options.was_supplied("synthcover"));

  if (!cb_explore_adf_option || !use_greedy) { return nullptr; }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
    options.insert("no_predict", "");
  }

  size_t problem_multiplier = 1;

  auto base = require_multiline(stack_builder.setup_base_learner());
  all.example_parser->lbl_parser = VW::cb_label_parser_global;

  using explore_type = cb_explore_adf_base<cb_explore_adf_greedy>;
  auto data = VW::make_unique<explore_type>(all.global_metrics.are_metrics_enabled(), epsilon, first_only);

  if (epsilon < 0.0 || epsilon > 1.0) { THROW("The value of epsilon must be in [0,1]"); }
  auto l = make_reduction_learner(std::move(data), base, explore_type::learn, explore_type::predict,
      stack_builder.get_setupfn_name(cb_explore_adf_greedy_setup))
               .set_learn_returns_prediction(base->learn_returns_prediction)
               .set_input_label_type(VW::label_type_t::CB)
               .set_output_label_type(VW::label_type_t::CB)
               .set_input_prediction_type(VW::prediction_type_t::ACTION_SCORES)
               .set_output_prediction_type(VW::prediction_type_t::ACTION_PROBS)
               .set_params_per_weight(problem_multiplier)
               .set_output_example_prediction(explore_type::output_example_prediction)
               .set_update_stats(explore_type::update_stats)
               .set_print_update(explore_type::print_update)
               .set_persist_metrics(explore_type::persist_metrics)
               .build();
  return l;
}
