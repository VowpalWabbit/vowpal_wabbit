#include "cb_explore_adf_bag.h"
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

namespace VW {
namespace cb_explore_adf {
namespace bag {

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

  m_scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) m_scores.push_back(0.f);
  m_top_actions.assign(num_actions, 0);
  for (uint32_t i = 0; i < m_bag_size; i++)
  {
    // avoid updates to the random num generator
    // for greedify, always update first policy once
    uint32_t count = is_learn ? ((m_greedify && i == 0) ? 1 : BS::weight_gen(*m_all)) : 0;

    if (is_learn && count > 0)
      LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset, i);
    else
      LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset, i);

    assert(preds.size() == num_actions);
    for (auto e : preds) m_scores[e.action] += e.score;

    if (!m_first_only)
    {
      size_t tied_actions = fill_tied(preds);
      for (size_t i = 0; i < tied_actions; ++i) m_top_actions[preds[i].action] += 1.f / tied_actions;
    }
    else
      m_top_actions[preds[0].action] += 1.f;
    if (is_learn)
      for (uint32_t j = 1; j < count; j++)
        LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset, i);
  }

  m_action_probs.clear();
  for (uint32_t i = 0; i < m_scores.size(); i++) m_action_probs.push_back({i, 0.});

  // generate distribution over actions
  exploration::generate_bag(
      begin(m_top_actions), end(m_top_actions), begin_scores(m_action_probs), end_scores(m_action_probs));

  exploration::enforce_minimum_probability(m_epsilon, true, begin_scores(m_action_probs), end_scores(m_action_probs));

  sort_action_probs(m_action_probs, m_scores);

  for (size_t i = 0; i < num_actions; i++) preds[i] = m_action_probs[i];
}

cb_explore_adf_bag::~cb_explore_adf_bag() {
  m_action_probs.delete_v();
}

template <bool is_learn>
void cb_explore_adf_bag::predict_or_learn(
    cb_explore_adf_bag& data, LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
    cb_explore_adf_base::learn(data, &cb_explore_adf_bag::predict_or_learn_impl<true>,
        &cb_explore_adf_bag::predict_or_learn_impl<false>, base, examples);
  else
    cb_explore_adf_base::predict(data, &cb_explore_adf_bag::predict_or_learn_impl<false>, base, examples);
}

void finish_multiline_example(vw& all, cb_explore_adf_bag& data, multi_ex& ec_seq)
{
  data.finish_multiline_example(all, ec_seq);
}

void finish(cb_explore_adf_bag& data) { data.~cb_explore_adf_bag(); }

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  auto data = scoped_calloc_or_throw<cb_explore_adf_bag>();
  bool cb_explore_adf_option = false;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", data->m_epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("bag", data->m_bag_size).keep().help("bagging-based exploration"))
      .add(make_option("greedify", data->m_greedify).keep().help("always update first policy once in bagging"))
      .add(make_option("first_only", data->m_first_only)
               .keep()
               .help("Only explore the first action in a tie-breaking event"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("bag"))
    return nullptr;

  data->m_all = &all;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Set explore_type
  size_t problem_multiplier = data->m_bag_size;

  LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  // Extract from lower level reductions.
  data->m_gen_cs.scorer = all.scorer;

  LEARNER::learner<cb_explore_adf_bag, multi_ex>& l =
      LEARNER::init_learner(data, base, cb_explore_adf_bag::predict_or_learn<true>,
          cb_explore_adf_bag::predict_or_learn<false>, problem_multiplier, prediction_type::action_probs);

  l.set_finish_example(finish_multiline_example);
  l.set_finish(finish);
  return make_base(l);
}

}  // namespace bag
}  // namespace cb_explore_adf
}  // namespace VW
