// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_softmax.h"
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace VW
{
namespace cb_explore_adf
{
namespace softmax
{
struct cb_explore_adf_softmax
{
 private:
  float _epsilon;
  float _lambda;

 public:
  cb_explore_adf_softmax(float epsilon, float lambda);
  ~cb_explore_adf_softmax() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_softmax::cb_explore_adf_softmax(float epsilon, float lambda) : _epsilon(epsilon), _lambda(lambda) {}

template <bool is_learn>
void cb_explore_adf_softmax::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  exploration::generate_softmax(
      -_lambda, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));

  exploration::enforce_minimum_probability(_epsilon, true, begin_scores(preds), end_scores(preds));
}

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool softmax = false;
  float epsilon = 0.;
  float lambda = 0.;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("softmax", softmax).keep().help("softmax exploration"))
      .add(make_option("lambda", lambda).keep().default_value(1.f).help("parameter for softmax"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !softmax)
    return nullptr;

  if (lambda < 0)  // Lambda should always be positive because we are using a cost basis.
    lambda = -lambda;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Set explore_type
  size_t problem_multiplier = 1;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_softmax>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, lambda);
  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}
}  // namespace softmax
}  // namespace cb_explore_adf
}  // namespace VW
