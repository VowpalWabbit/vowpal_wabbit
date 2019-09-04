/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once
#include <stdint.h>
#include <algorithm>

// Most of these includes are required because templated functions are using the objects defined in them
// A few options to get rid of them:
// - Use virtual function calls in predict/learn to get rid of the templates entirely (con: virtual function calls)
// - Cut out the portions of code that actually use the objects and put them into new functions
//   defined in the cc file (con: can't inline those functions)
// - templatize all input parameters (con: no type safety)
#include "v_array.h"         // required by action_score.h
#include "action_score.h"    // used in sort_action_probs
#include "cb.h"              // required for CB::label
#include "cb_adf.h"          // used for function call in predict/learn
#include "example.h"         // used in predict
#include "gen_cs_example.h"  // required for GEN_CS::cb_to_cs_adf
#include "reductions_fwd.h"

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
  using PredictLearnFn = void (T::*)(LEARNER::multi_learner& base, multi_ex& examples);

  template <typename T>
  void predict(T& data, PredictLearnFn<T> predict_p, LEARNER::multi_learner& base, multi_ex& examples);

  template <typename T>
  void learn(T& data, PredictLearnFn<T> learn_p, PredictLearnFn<T> predict_p, LEARNER::multi_learner& base,
      multi_ex& examples);

 private:
  // used in output_example
  CB::label m_action_label;
  CB::label m_empty_label;

 protected:
  size_t fill_tied(v_array<ACTION_SCORE::action_score>& preds);

  void sort_action_probs(v_array<ACTION_SCORE::action_score>& probs, const std::vector<float>& scores);

 private:
  void output_example_seq(vw& all, multi_ex& ec_seq);
  void output_example(vw& all, multi_ex& ec_seq);
};

template <typename T>
using PredictLearnFn = void (T::*)(LEARNER::multi_learner& base, multi_ex& examples);

template <typename T>
inline void cb_explore_adf_base::predict(
    T& data, PredictLearnFn<T> predict_p, LEARNER::multi_learner& base, multi_ex& examples)
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
inline void cb_explore_adf_base::learn(
    T& data, PredictLearnFn<T> learn_p, PredictLearnFn<T> predict_p, LEARNER::multi_learner& base, multi_ex& examples)
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

inline void cb_explore_adf_base::sort_action_probs(
    v_array<ACTION_SCORE::action_score>& probs, const std::vector<float>& scores)
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

}  // namespace cb_explore_adf
}  // namespace VW
