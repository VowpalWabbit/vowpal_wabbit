// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_greedy.h"
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
#include <functional>

namespace VW
{
namespace cb_explore_adf
{
namespace greedy
{
struct cb_explore_adf_greedy
{
 private:
  float _epsilon;
  bool _first_only;

 public:
  cb_explore_adf_greedy(float epsilon, bool first_only);
  ~cb_explore_adf_greedy() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_greedy::cb_explore_adf_greedy(float epsilon, bool first_only)
    : _epsilon(epsilon), _first_only(first_only)
{
}

template <bool is_learn>
void cb_explore_adf_greedy::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  // Explore uniform random an epsilon fraction of the time.
  LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  ACTION_SCORE::action_scores& preds = examples[0]->pred.a_s;

  uint32_t num_actions = (uint32_t)preds.size();

  size_t tied_actions = fill_tied(preds);

  const float prob = _epsilon / num_actions;
  for (size_t i = 0; i < num_actions; i++) preds[i].score = prob;
  if (!_first_only)
  {
    for (size_t i = 0; i < tied_actions; ++i) preds[i].score += (1.f - _epsilon) / tied_actions;
  }
  else
    preds[0].score += 1.f - _epsilon;
}

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  bool first_only = false;

  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"));
  options.add_and_parse(new_options);

  // NOTE: epsilon-greedy is the default explore type. This basically runs if none of the other explore strategies are
  // used
  bool use_greedy = !(options.was_supplied("first") || options.was_supplied("bag") || options.was_supplied("cover") ||
      options.was_supplied("regcb") || options.was_supplied("regcbopt") || options.was_supplied("softmax"));

  if (!cb_explore_adf_option || !use_greedy)
    return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  size_t problem_multiplier = 1;

  if (!options.was_supplied("epsilon"))
    epsilon = 0.05f;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_greedy>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, first_only);

  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}

}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
