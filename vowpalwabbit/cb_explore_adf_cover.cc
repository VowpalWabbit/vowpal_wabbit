#include "cb_explore_adf_cover.h"
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
namespace cover
{
cb_explore_adf_cover::cb_explore_adf_cover(size_t cover_size, float psi, bool nounif, bool first_only,
    LEARNER::multi_learner* cs_ldf_learner, LEARNER::single_learner* scorer, size_t cb_type)
    : m_cover_size(cover_size), m_psi(psi), m_nounif(nounif), m_first_only(first_only), m_cs_ldf_learner(cs_ldf_learner)
{
  m_gen_cs.cb_type = cb_type;
  m_gen_cs.scorer = scorer;
}

template <bool is_learn>
void cb_explore_adf_cover::predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples)
{
  m_gen_cs.known_cost = m_known_cost;
  // Randomize over predictions from a base set of predictors
  // Use cost sensitive oracle to cover actions to form distribution.
  const bool is_mtr = m_gen_cs.cb_type == CB_TYPE_MTR;
  if (is_learn)
  {
    if (is_mtr)  // use DR estimates for non-ERM policies in MTR
      GEN_CS::gen_cs_example_dr<true>(m_gen_cs, examples, m_cs_labels);
    else
      GEN_CS::gen_cs_example<false>(m_gen_cs, examples, m_cs_labels);
    LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
  }
  else
  {
    GEN_CS::gen_cs_example_ips(examples, m_cs_labels);
    LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);
  }
  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  const uint32_t num_actions = (uint32_t)preds.size();

  float additive_probability = 1.f / (float)m_cover_size;
  const float min_prob = (std::min)(1.f / num_actions, 1.f / (float)std::sqrt(m_counter * num_actions));
  m_action_probs.clear();
  for (uint32_t i = 0; i < num_actions; i++) m_action_probs.push_back({i, 0.});
  m_scores.clear();
  for (uint32_t i = 0; i < num_actions; i++) m_scores.push_back(preds[i].score);

  if (!m_first_only)
  {
    size_t tied_actions = fill_tied(preds);
    for (size_t i = 0; i < tied_actions; ++i)
      m_action_probs[preds[i].action].score += additive_probability / tied_actions;
  }
  else
    m_action_probs[preds[0].action].score += additive_probability;

  float norm = min_prob * num_actions + (additive_probability - min_prob);
  for (size_t i = 1; i < m_cover_size; i++)
  {
    // Create costs of each action based on online cover
    if (is_learn)
    {
      m_cs_labels_2.costs.clear();
      for (uint32_t j = 0; j < num_actions; j++)
      {
        float pseudo_cost = m_cs_labels.costs[j].x - m_psi * min_prob / ((std::max)(m_action_probs[j].score, min_prob) / norm);
        m_cs_labels_2.costs.push_back({pseudo_cost, j, 0., 0.});
      }
      GEN_CS::call_cs_ldf<true>(*(m_cs_ldf_learner), examples, m_cb_labels, m_cs_labels_2, m_prepped_cs_labels,
          examples[0]->ft_offset, i + 1);
    }
    else
      GEN_CS::call_cs_ldf<false>(
          *(m_cs_ldf_learner), examples, m_cb_labels, m_cs_labels, m_prepped_cs_labels, examples[0]->ft_offset, i + 1);

    for (uint32_t i = 0; i < num_actions; i++) m_scores[i] += preds[i].score;
    if (!m_first_only)
    {
      size_t tied_actions = fill_tied(preds);
      const float add_prob = additive_probability / tied_actions;
      for (size_t i = 0; i < tied_actions; ++i)
      {
        if (m_action_probs[preds[i].action].score < min_prob)
          norm += (std::max)(0.f, add_prob - (min_prob - m_action_probs[preds[i].action].score));
        else
          norm += add_prob;
        m_action_probs[preds[i].action].score += add_prob;
      }
    }
    else
    {
      uint32_t action = preds[0].action;
      if (m_action_probs[action].score < min_prob)
        norm += (std::max)(0.f, additive_probability - (min_prob - m_action_probs[action].score));
      else
        norm += additive_probability;
      m_action_probs[action].score += additive_probability;
    }
  }

  exploration::enforce_minimum_probability(
      min_prob * num_actions, !m_nounif, begin_scores(m_action_probs), end_scores(m_action_probs));

  sort_action_probs(m_action_probs, m_scores);
  for (size_t i = 0; i < num_actions; i++) preds[i] = m_action_probs[i];

  if (is_learn)
    ++m_counter;
}

cb_explore_adf_cover::~cb_explore_adf_cover()
{
  m_cb_labels.delete_v();
  for (size_t i = 0; i < m_prepped_cs_labels.size(); i++) m_prepped_cs_labels[i].costs.delete_v();
  m_prepped_cs_labels.delete_v();
  m_cs_labels_2.costs.delete_v();
  m_cs_labels.costs.delete_v();
  m_action_probs.delete_v();
  m_gen_cs.pred_scores.costs.delete_v();
}

template <bool is_learn>
void cb_explore_adf_cover::predict_or_learn(
    cb_explore_adf_cover& data, LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
    data.learn(data, &cb_explore_adf_cover::predict_or_learn_impl<true>,
        &cb_explore_adf_cover::predict_or_learn_impl<false>, base, examples);
  else
    data.predict(data, &cb_explore_adf_cover::predict_or_learn_impl<false>, base, examples);
}

void finish_multiline_example(vw& all, cb_explore_adf_cover& data, multi_ex& ec_seq)
{
  data.finish_multiline_example(all, ec_seq);
}

LEARNER::base_learner* setup(config::options_i& options, vw& all)
{
  using config::make_option;

  bool cb_explore_adf_option = false;
  std::string type_string = "mtr";
  size_t cover_size;
  float psi;
  bool nounif;
  bool first_only;

  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("cover", cover_size).keep().help("Online cover based exploration"))
      .add(make_option("psi", psi).keep().default_value(1.0f).help("disagreement parameter for cover"))
      .add(make_option("nounif", nounif)
               .keep()
               .help("do not explore uniformly on zero-probability actions in cover"))
      .add(make_option("first_only", first_only)
               .keep()
               .help("Only explore the first action in a tie-breaking event"))
      .add(make_option("cb_type", type_string)
               .keep()
               .help("contextual bandit method to use in {ips,dr,mtr}. Default: mtr"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("cover"))
    return nullptr;

  // Ensure serialization of cb_type in all cases.
  if (!options.was_supplied("cb_type"))
  {
    options.insert("cb_type", type_string);
    options.add_and_parse(new_options);
  }

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Set cb_type
  size_t cb_type_enum;
  if (type_string.compare("dr") == 0)
    cb_type_enum = CB_TYPE_DR;
  else if (type_string.compare("ips") == 0)
    cb_type_enum = CB_TYPE_IPS;
  else if (type_string.compare("mtr") == 0)
  {
    all.trace_message << "warning: currently, mtr is only used for the first policy in cover, other policies use dr"
                      << std::endl;
    cb_type_enum = CB_TYPE_MTR;
  }
  else
  {
    all.trace_message << "warning: cb_type must be in {'ips','dr','mtr'}; resetting to mtr." << std::endl;
    options.replace("cb_type", "mtr");
    cb_type_enum = CB_TYPE_MTR;
  }

  // Set explore_type
  size_t problem_multiplier = cover_size + 1;

  LEARNER::multi_learner* base = LEARNER::as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type::cb;

  auto data = scoped_calloc_or_throw<cb_explore_adf_cover>(
      cover_size, psi, nounif, first_only, as_multiline(all.cost_sensitive), all.scorer, cb_type_enum);

  LEARNER::learner<cb_explore_adf_cover, multi_ex>& l =
      init_learner(data, base, cb_explore_adf_cover::predict_or_learn<true>,
          cb_explore_adf_cover::predict_or_learn<false>, problem_multiplier, prediction_type::action_probs);

  l.set_finish_example(finish_multiline_example);
  return make_base(l);
}
}  // namespace cover
}  // namespace cb_explore_adf
}  // namespace VW
