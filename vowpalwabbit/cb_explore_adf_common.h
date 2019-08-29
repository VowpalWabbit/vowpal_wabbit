/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once
#include <stdint.h>
#include "v_array.h"

#include "learner.h"
#include "action_score.h"
#include "cb.h"
#include "cb_adf.h"
#include "example.h"
#include "gen_cs_example.h"
#include <algorithm>

namespace VW
{
namespace cb_explore_adf
{
// data common to all cb_explore_adf reductions
struct cb_explore_adf_base
{
 public:
  GEN_CS::cb_to_cs_adf m_gen_cs;

 public:
  void finish_multiline_example(vw& all, multi_ex& ec_seq);
  virtual ~cb_explore_adf_base();

  template <typename T>
  using Fn = void (T::*)(LEARNER::multi_learner& base, multi_ex& examples);

  template <typename T>
  static void predict(T& data, Fn<T> predict_p, LEARNER::multi_learner& base, multi_ex& examples)
  {
    example* label_example = CB_ADF::test_adf_sequence(examples);
    data.m_gen_cs.known_cost = CB_ADF::get_observed_cost(examples);

    if (label_example != nullptr)
    {
      // predict path, replace the label example with an empty one
      data.m_action_label = label_example->l.cb;
      label_example->l.cb = data.m_empty_label;
    }

    (data.*predict_p)(base, examples);

    if (label_example != nullptr)
    {
      // predict path, restore label
      label_example->l.cb = data.m_action_label;
    }
  }
  template <typename T>
  static void learn(T& data, Fn<T> learn_p, Fn<T> predict_p, LEARNER::multi_learner& base,
      multi_ex& examples)
  {
    example* label_example = CB_ADF::test_adf_sequence(examples);
    if (label_example != nullptr)
    {
      data.m_gen_cs.known_cost = CB_ADF::get_observed_cost(examples);
      // learn iff label_example != nullptr
      (data.*learn_p)(base, examples);
    }
    else
    {
       predict(data, predict_p, base, examples);
    }
  }

 private:
  // used in output_example
  CB::label m_action_label;
  CB::label m_empty_label;

 protected:
  size_t fill_tied(v_array<ACTION_SCORE::action_score>& preds);

  void sort_action_probs(v_array<ACTION_SCORE::action_score>& probs, const std::vector<float>& scores)
  {
    // We want to preserve the score order in the returned action_probs if possible.  To do this,
    // sort top_actions and action_probs by the order induced in scores.
    std::sort(probs.begin(), probs.end(),
        [&scores](const ACTION_SCORE::action_score& as1, const ACTION_SCORE::action_score& as2) {
          if (as1.score > as2.score)
            return true;
          else if (as1.score < as2.score)
            return false;
          // equal probabilities
          if (scores[as1.action] < scores[as2.action])
            return true;
          else if (scores[as1.action] > scores[as2.action])
            return false;
          // equal probabilities and equal cost estimates
          return as1.action < as2.action;
        });
  }

 private:
  void output_example_seq(vw& all, multi_ex& ec_seq);
  void output_example(vw& all, multi_ex& ec_seq);
};

}  // namespace cb_explore_adf
}  // namespace VW
