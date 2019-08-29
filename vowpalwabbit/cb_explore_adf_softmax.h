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

namespace VW
{
namespace cb_explore_adf
{
namespace softmax
{
LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

struct cb_explore_adf_softmax : public cb_explore_adf_base
{
 public:
  float m_epsilon;
  float m_lambda;

 public:
  template <bool is_learn>
  static void predict_or_learn(cb_explore_adf_softmax& data, LEARNER::multi_learner& base, multi_ex& examples);
  ~cb_explore_adf_softmax() = default;

 private:
  template <bool is_learn>
  void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
};
}  // namespace softmax
}  // namespace cb_explore_adf
}  // namespace VW
