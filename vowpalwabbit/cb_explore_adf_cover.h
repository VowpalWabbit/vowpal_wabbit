/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include <stdint.h>
#include <vector>

#include "cb.h"
#include "cb_explore_adf_common.h"
#include "cost_sensitive.h"
#include "learner.h"
#include "options.h"
#include "v_array.h"

#include "action_score.h"

namespace VW
{
namespace cb_explore_adf
{
namespace cover
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_cover : public cb_explore_adf_base
{
 public:
  v_array<ACTION_SCORE::action_score> m_action_probs;
  std::vector<float> m_scores;

  size_t m_cover_size;
  float m_psi;
  bool m_nounif;
  bool m_first_only;
  size_t m_counter;

  LEARNER::multi_learner* m_cs_ldf_learner;

  COST_SENSITIVE::label m_cs_labels;
  COST_SENSITIVE::label m_cs_labels_2;
  v_array<COST_SENSITIVE::label> m_prepped_cs_labels;
  v_array<CB::label> m_cb_labels;

 public:
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_cover& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_cover();

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};

}  // namespace cover
}  // namespace cb_explore_adf
}  // namespace VW
