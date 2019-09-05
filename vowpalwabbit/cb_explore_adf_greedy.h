/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#pragma once

#include "cb_explore_adf_common.h"
#include "reductions_fwd.h"

#include <vector>

namespace VW
{
namespace cb_explore_adf
{
namespace greedy
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_greedy : public cb_explore_adf_base
{
 private:
  float m_epsilon;
  bool m_first_only;

 public:
  cb_explore_adf_greedy(float epsilon, bool first_only);

  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_greedy& data, LEARNER::multi_learner& base, multi_ex& examples);
  cb_explore_adf_greedy() = default;
  ~cb_explore_adf_greedy() = default;

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace greedy
}  // namespace cb_explore_adf
}  // namespace VW
