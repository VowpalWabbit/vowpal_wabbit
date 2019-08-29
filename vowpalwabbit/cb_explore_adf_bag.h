/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include "cb_explore_adf_common.h"
#include "example.h"
#include "global_data.h"
#include "learner.h"
#include "options.h"

#include <vector>

namespace VW {
namespace cb_explore_adf {
namespace bag {
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_bag : public cb_explore_adf_base
{
 public:
  float m_epsilon;
  size_t m_bag_size;
  bool m_greedify;
  bool m_first_only;
  vw* m_all;

  v_array<ACTION_SCORE::action_score> m_action_probs;
  std::vector<float> m_scores;
  std::vector<float> m_top_actions;

 public:
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_bag& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_bag();

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace first
}  // namespace cb_explore_adf
}  // namespace VW
