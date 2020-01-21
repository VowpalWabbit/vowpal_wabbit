// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_first.h"

#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include "cb_explore_adf_common.h"
#include <vector>
#include <algorithm>
#include <cmath>

namespace VW
{
namespace cb_explore_adf
{
namespace first
{
struct cb_explore_adf_first
{
 private:
  size_t _tau;
  float _epsilon;

 public:
  cb_explore_adf_first(size_t tau, float epsilon);
  ~cb_explore_adf_first() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_first::cb_explore_adf_first(size_t tau, float epsilon) : _tau(tau), _epsilon(epsilon) {}

template <bool is_learn>
void cb_explore_adf_first::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  // Explore tau times, then act according to optimal.
  if (is_learn)
    LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
  else
    LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();

  if (_tau)
  {
    float prob = 1.f / (float)num_actions;
    for (size_t i = 0; i < num_actions; i++) preds[i].score = prob;
    if (is_learn)
      _tau--;
  }
  else
  {
    for (size_t i = 1; i < num_actions; i++) preds[i].score = 0.;
    preds[0].score = 1.0;
  }

  exploration::enforce_minimum_probability(_epsilon, true, begin_scores(preds), end_scores(preds));
}

LEARNER::base_learner* setup(config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  size_t tau = 0;
  float epsilon = 0.;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("first", tau).keep().help("tau-first exploration"))
      .add(make_option("epsilon", epsilon).keep().help("epsilon-greedy exploration"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("first"))
    return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  size_t problem_multiplier = 1;

  LEARNER::multi_learner* base = LEARNER::as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_first>;
  auto data = scoped_calloc_or_throw<explore_type>(tau, epsilon);

  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}
}  // namespace first
}  // namespace cb_explore_adf
}  // namespace VW
