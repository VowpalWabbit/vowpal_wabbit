// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_bag.h"

#include "cb_explore_adf_common.h"
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

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

namespace VW
{
namespace cb_explore_adf
{
namespace bag
{
struct cb_explore_adf_bag
{
 private:
  float _epsilon;
  size_t _bag_size;
  bool _greedify;
  bool _first_only;
  std::shared_ptr<rand_state> _random_state;

  v_array<ACTION_SCORE::action_score> _action_probs;
  std::vector<float> _scores;
  std::vector<float> _top_actions;

 public:
  cb_explore_adf_bag(
      float epsilon, size_t bag_size, bool greedify, bool first_only, std::shared_ptr<rand_state> random_state);
  ~cb_explore_adf_bag();

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_bag::cb_explore_adf_bag(
    float epsilon, size_t bag_size, bool greedify, bool first_only, std::shared_ptr<rand_state> random_state)
    : _epsilon(epsilon), _bag_size(bag_size), _greedify(greedify), _first_only(first_only), _random_state(random_state)
{
}

template <bool is_learn>
void cb_explore_adf_bag::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  // Randomize over predictions from a base set of predictors
  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)examples.size();
  if (num_actions == 0)
  {
    preds.clear();
    return;
  }

  _scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) _scores.push_back(0.f);
  _top_actions.assign(num_actions, 0);
  for (uint32_t i = 0; i < _bag_size; i++)
  {
    // avoid updates to the random num generator
    // for greedify, always update first policy once
    uint32_t count = is_learn ? ((_greedify && i == 0) ? 1 : BS::weight_gen(_random_state)) : 0;

    if (is_learn && count > 0)
      LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset, i);
    else
      LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset, i);

    assert(preds.size() == num_actions);
    for (auto e : preds) _scores[e.action] += e.score;

    if (!_first_only)
    {
      size_t tied_actions = fill_tied(preds);
      for (size_t i = 0; i < tied_actions; ++i) _top_actions[preds[i].action] += 1.f / tied_actions;
    }
    else
      _top_actions[preds[0].action] += 1.f;
    if (is_learn)
      for (uint32_t j = 1; j < count; j++)
        LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset, i);
  }

  _action_probs.clear();
  for (uint32_t i = 0; i < _scores.size(); i++) _action_probs.push_back({i, 0.});

  // generate distribution over actions
  exploration::generate_bag(
      begin(_top_actions), end(_top_actions), begin_scores(_action_probs), end_scores(_action_probs));

  exploration::enforce_minimum_probability(_epsilon, true, begin_scores(_action_probs), end_scores(_action_probs));

  sort_action_probs(_action_probs, _scores);

  for (size_t i = 0; i < num_actions; i++) preds[i] = _action_probs[i];
}

cb_explore_adf_bag::~cb_explore_adf_bag() { _action_probs.delete_v(); }

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  size_t bag_size = 0;
  bool greedify = false;
  bool first_only = false;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("bag", bag_size).keep().help("bagging-based exploration"))
      .add(make_option("greedify", greedify).keep().help("always update first policy once in bagging"))
      .add(make_option("first_only", first_only).keep().help("Only explore the first action in a tie-breaking event"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("bag"))
    return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  size_t problem_multiplier = bag_size;
  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_bag>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, bag_size, greedify, first_only, all.get_random_state());

  LEARNER::learner<explore_type, multi_ex>& l = LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}

}  // namespace bag
}  // namespace cb_explore_adf
}  // namespace VW
