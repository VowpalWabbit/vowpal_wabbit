// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_synthcover.h"

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
namespace synthcover
{
struct cb_explore_adf_synthcover
{
private:
  float _epsilon;
  float _psi;
  float _eta;
  size_t _synthcoversize;
  std::shared_ptr<rand_state> _random_state;

  v_array<ACTION_SCORE::action_score> _action_probs;
  std::vector<float> beta;
  std::vector<int> policyaction;
  float min_cost;
  float max_cost;

public:
  cb_explore_adf_synthcover(
      float epsilon, float psi, float eta, size_t synthcoversize, std::shared_ptr<rand_state> random_state);
  ~cb_explore_adf_synthcover();

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

//  float& get_min_cost() { return min_cost; }
//  float& get_max_cost() { return max_cost; }

private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);
};

cb_explore_adf_synthcover::cb_explore_adf_synthcover(
      float epsilon, float psi, float eta, size_t synthcoversize, std::shared_ptr<rand_state> random_state)
    : _epsilon(epsilon), _psi(psi), _eta(eta), _synthcoversize(synthcoversize), _random_state(random_state), beta(synthcoversize, 1.0 / synthcoversize), min_cost(0.0), max_cost(0.0)
{
}

template <bool is_learn>
void cb_explore_adf_synthcover::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  VW::LEARNER::multiline_learn_or_predict<is_learn>(base, examples, examples[0]->ft_offset);

  const auto it =
      std::find_if(examples.begin(), examples.end(), [](example *item) { return !item->l.cb.costs.empty(); });

  if (it != examples.end())
  {
    const CB::cb_class logged = (*it)->l.cb.costs[0];

    min_cost = std::min(logged.cost, min_cost);
    max_cost = std::max(logged.cost, max_cost);
  }

  ACTION_SCORE::action_scores& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)examples.size();
  if (num_actions == 0)
  {
    preds.clear();
    return;
  }
  for (size_t i = 0; i < num_actions; i++)
  {
    preds[i].score = std::min(max_cost, std::max(min_cost, preds[i].score));
  }
  std::make_heap(preds.begin(), preds.end(),
                 [](const ACTION_SCORE::action_score &a, const ACTION_SCORE::action_score &b) {
                 return ACTION_SCORE::score_comp(&a, &b) > 0;
              });

  _action_probs.clear();
  for (uint32_t i = 0; i < num_actions; i++) _action_probs.push_back({i, 0.});

  policyaction.clear();
  for (uint32_t i = 0; i < _synthcoversize; i++)
  {
    std::pop_heap(preds.begin(), preds.end(),
                  [](const ACTION_SCORE::action_score &a, const ACTION_SCORE::action_score &b) {
                  return ACTION_SCORE::score_comp(&a, &b) > 0;
               });
    // NB: what STL calls pop_back(), v_array calls pop().  facepalm.
    auto minpred = preds.pop();
    policyaction.push_back(minpred.action);
    _action_probs[minpred.action].score += beta[i];
    minpred.score += beta[i] * _psi;
    preds.push_back(minpred);
    std::push_heap(preds.begin(), preds.end(),
                  [](const ACTION_SCORE::action_score &a, const ACTION_SCORE::action_score &b) {
                  return ACTION_SCORE::score_comp(&a, &b) > 0;
               });
  }

  exploration::enforce_minimum_probability(_epsilon, true, begin_scores(_action_probs), end_scores(_action_probs));

  if (is_learn)
  {
    float sumbeta = 0.0;
    for (size_t i = 0; i < _synthcoversize; i++)
    {
      beta[i] *= 1 + _eta / _action_probs[policyaction[i]].score;
      sumbeta += beta[i];
    }
    for (size_t i = 0; i < _synthcoversize; i++)
    {
      beta[i] /= sumbeta;
    }
  }

  std::sort(_action_probs.begin(), _action_probs.end(),
            [](const ACTION_SCORE::action_score &a, const ACTION_SCORE::action_score &b) {
            return ACTION_SCORE::score_comp(&a, &b) > 0;
         });

  for (size_t i = 0; i < num_actions; i++) preds[i] = _action_probs[i];
}

cb_explore_adf_synthcover::~cb_explore_adf_synthcover() { _action_probs.delete_v(); }

// TODO: wait for Olga's changes to merge
//
// void cb_explore_adf_synthcover::save_load(io_buf& model_file, bool read, bool text)
// {
//   std::stringstream msg;
//   msg << "min_cost " << c.explore.get_min_cost() << "\n";
//   bin_text_read_write_fixed(
//       model_file, (char*)&c.explore.get_min_cost(), sizeof(c.explore.get_min_cost()), "", read, msg, text);
//
//   msg << "max_cost " << c.explore.get_max_cost() << "\n";
//   bin_text_read_write_fixed(
//       model_file, (char*)&c.explore.get_max_cost(), sizeof(c.explore.get_max_cost()), "", read, msg, text);
// }

VW::LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  float epsilon = 0.;
  size_t synthcoversize;
  bool use_synthcover = false;
  float psi;
  float eta;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .necessary()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("epsilon", epsilon).keep().allow_override().help("epsilon-greedy exploration"))
      .add(make_option("synthcover", use_synthcover).keep().necessary().help("use synthetic cover exploration"))
      .add(make_option("synthcoverpsi", psi).keep().default_value(0.1).allow_override().help("exploration reward bonus"))
      .add(make_option("synthcovereta", eta).keep().default_value(0.2).allow_override().help("analytic center learning rate"))
      .add(make_option("synthcoversize", synthcoversize).keep().default_value(100).allow_override().help("analytic center learning rate"));

  if (!options.add_parse_and_check_necessary(new_options)) return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf")) { options.insert("cb_adf", ""); }

  if (synthcoversize <= 0) { THROW("synthcoversize must be >= 1"); }
  if (epsilon < 0) { THROW("epsilon must be non-negative"); }
  if (psi <= 0) { THROW("synthcoverpsi must be positive"); }
  if (eta <= 0 || eta > 0.25) { THROW("synthcovereta must be in [0, 0.25]"); }

  if (!all.logger.quiet)
  {
    std::cerr << "Using synthcover for CB exploration" << std::endl;
    std::cerr << "synthcoversize = " << synthcoversize << std::endl;
    if (epsilon > 0) std::cerr << "epsilon = " << epsilon << std::endl;
    std::cerr << "synthcoverpsi = " << psi << std::endl;
    std::cerr << "synthcovereta = " << eta << std::endl;
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  size_t problem_multiplier = 1;
  VW::LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.example_parser->lbl_parser = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_synthcover>;
  auto data = scoped_calloc_or_throw<explore_type>(epsilon, psi, eta, synthcoversize, all.get_random_state());

  VW::LEARNER::learner<explore_type, multi_ex>& l = VW::LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  //l.set_save_load(explore_type::save_load);
  return make_base(l);
}

}  // namespace synthcover
}  // namespace cb_explore_adf
}  // namespace VW
