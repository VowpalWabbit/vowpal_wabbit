// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "cb_explore_adf_squarecb.h"
#include "reductions.h"
#include "cb_adf.h"
#include "rand48.h"
#include "bs.h"
#include "gen_cs_example.h"
#include "cb_explore.h"
#include "explore.h"
#include "action_score.h"
#include "cb.h"
#include <vector>
#include <algorithm>
#include <cmath>

/* Debugging */
#include <iostream>

// This file implements the SquareCB algorithm/reduction (Foster and Rakhlin, 2020, https://arxiv.org/abs/2002.04926), with the VW learner as the base algorithm.

// All exploration algorithms return a vector of id, probability tuples, sorted in order of scores. The probabilities
// are the probability with which each action should be replaced to the top of the list.

namespace VW
{
namespace cb_explore_adf
{
namespace squarecb
{
struct cb_explore_adf_squarecb
{
 private:
  // size_t _counter;
  size_t _counter;
  float _gamma;	// Greediness parameter.

 public:
  cb_explore_adf_squarecb(float gamma);
  ~cb_explore_adf_squarecb() = default;

  // Should be called through cb_explore_adf_base for pre/post-processing
  void predict(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<false>(base, examples); }
  void learn(VW::LEARNER::multi_learner& base, multi_ex& examples) { predict_or_learn_impl<true>(base, examples); }

 private:
  template <bool is_learn>
  void predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples);

};

cb_explore_adf_squarecb::cb_explore_adf_squarecb(
    float gamma)
  : _counter(0), _gamma(gamma)
{
}


template <bool is_learn>
void cb_explore_adf_squarecb::predict_or_learn_impl(VW::LEARNER::multi_learner& base, multi_ex& examples)
{
  if (is_learn)
  {
    for (size_t i = 0; i < examples.size() - 1; ++i)
    {
      CB::label& ld = examples[i]->l.cb;
      if (ld.costs.size() == 1)
        ld.costs[0].probability = 1.f;  // no importance weighting
    }

    VW::LEARNER::multiline_learn_or_predict<true>(base, examples, examples[0]->ft_offset);
    ++_counter;
  }
  else
    VW::LEARNER::multiline_learn_or_predict<false>(base, examples, examples[0]->ft_offset);

  v_array<ACTION_SCORE::action_score>& preds = examples[0]->pred.a_s;
  uint32_t num_actions = (uint32_t)preds.size();
  const float multiplier = _gamma*pow(_counter, .25f);

  if (!is_learn)
    {
      size_t a_min = 0;
      float min_cost = preds[0].score;
      for(size_t a = 0; a < num_actions; ++a)
	{
	  if(preds[a].score < min_cost)
	    {
	      a_min = a;
	      min_cost = preds[a].score;
	    }
	}
      float total_weight = 0;
      float pa = 0;
      for(size_t a = 0; a < num_actions; ++a)
	{
	  if (a == a_min)
	    continue;
	  pa = 1./(num_actions + multiplier*(preds[a].score-min_cost));
	  preds[a].score = pa;
	  total_weight += pa;
	}
      preds[a_min].score = 1.f-total_weight;

   }

}

VW::LEARNER::base_learner* setup(VW::config::options_i& options, vw& all)
{
  using config::make_option;
  bool cb_explore_adf_option = false;
  bool squarecb = false;
  const std::string mtr = "mtr";
  std::string type_string(mtr);
  float gamma = 1.;
  config::option_group_definition new_options("Contextual Bandit Exploration with Action Dependent Features");
  new_options
      .add(make_option("cb_explore_adf", cb_explore_adf_option)
               .keep()
               .help("Online explore-exploit for a contextual bandit problem with multiline action dependent features"))
      .add(make_option("squarecb", squarecb).keep().help("SquareCB exploration"))
      .add(make_option("gamma", gamma).keep().default_value(1.f).help("SquareCB greediness parameter. Default = 1.0"))
      .add(make_option("cb_type", type_string)
               .keep()
               .help("contextual bandit method to use in {ips,dr,mtr}. Default: mtr"));
  options.add_and_parse(new_options);

  if (!cb_explore_adf_option || !options.was_supplied("squarecb"))
    return nullptr;

  // Ensure serialization of cb_adf in all cases.
  if (!options.was_supplied("cb_adf"))
  {
    options.insert("cb_adf", "");
  }
  if (type_string != mtr)
  {
    all.trace_message << "warning: bad cb_type, SquareCB only supports mtr; resetting to mtr." << std::endl;
    options.replace("cb_type", mtr);
  }

  all.delete_prediction = ACTION_SCORE::delete_action_scores;

  // Set explore_type
  size_t problem_multiplier = 1;

  VW::LEARNER::multi_learner* base = as_multiline(setup_base(options, all));
  all.p->lp = CB::cb_label;
  all.label_type = label_type_t::cb;

  using explore_type = cb_explore_adf_base<cb_explore_adf_squarecb>;
  auto data = scoped_calloc_or_throw<explore_type>(gamma);
  VW::LEARNER::learner<explore_type, multi_ex>& l = VW::LEARNER::init_learner(
      data, base, explore_type::learn, explore_type::predict, problem_multiplier, prediction_type_t::action_probs);

  l.set_finish_example(explore_type::finish_multiline_example);
  return make_base(l);
}

}  // namespace squarecb
}  // namespace cb_explore_adf
}  // namespace VW
