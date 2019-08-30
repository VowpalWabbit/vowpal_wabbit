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
template <bool is_learn>
void cb_explore_adf_softmax::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  exploration::generate_softmax(-m_lambda, begin_scores(preds), end_scores(preds), begin_scores(preds), end_scores(preds));

  exploration::enforce_minimum_probability(m_epsilon, true, begin_scores(preds), end_scores(preds));
}

template <bool is_learn>
void cb_explore_adf_softmax::predict_or_learn(
    cb_explore_adf_softmax& data, LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
    data.learn(data, &cb_explore_adf_softmax::predict_or_learn_impl<true>,
        &cb_explore_adf_softmax::predict_or_learn_impl<false>, base, examples);
  else
    data.predict(data, &cb_explore_adf_softmax::predict_or_learn_impl<false>, base, examples);
}

void finish_multiline_example(vw& all, cb_explore_adf_softmax& data, multi_ex& ec_seq)
{
  data.finish_multiline_example(all, ec_seq);
}

void finish(cb_explore_adf_softmax& data) { data.~cb_explore_adf_softmax(); }

LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  auto data = scoped_calloc_or_throw<cb_explore_adf_softmax>();
  bool cb_explore_adf_option = false;
  bool softmax = false;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", data->m_epsilon).keep().help("epsilon-greedy exploration"))
      .add(make_option("softmax", softmax).keep().help("softmax exploration"))
      .add(make_option("lambda", data->m_lambda).keep().default_value(1.f).help("parameter for softmax"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !softmax)
    return nullptr;

  if (data->m_lambda < 0)  // Lambda should always be positive because we are using a cost basis.
    data->m_lambda = -data->m_lambda;

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
  all.label_type = label_type::cb;

  // Extract from lower level reductions.
  data->m_gen_cs.scorer = all.scorer;

  LEARNER::learner<cb_explore_adf_softmax, multi_ex>& l =
      LEARNER::init_learner(data, base, cb_explore_adf_softmax::predict_or_learn<true>,
          cb_explore_adf_softmax::predict_or_learn<false>, problem_multiplier, prediction_type::action_probs);

  l.set_finish_example(finish_multiline_example);
  l.set_finish(finish);
  return make_base(l);
}
}  // namespace softmax
}  // namespace cb_explore_adf
}  // namespace VW
