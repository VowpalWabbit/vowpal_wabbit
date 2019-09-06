/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include "cb_explore_adf_common.h"
#include "reductions_fwd.h"

#include <vector>
#include <memory>

struct rand_state;

namespace VW
{
namespace cb_explore_adf
{
namespace bag
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_bag : public cb_explore_adf_base
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
  cb_explore_adf_bag(float epsilon, size_t bag_size, bool greedify, bool first_only, std::shared_ptr<rand_state> random_state);
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_bag& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_bag();

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace bag
}  // namespace cb_explore_adf
}  // namespace VW
