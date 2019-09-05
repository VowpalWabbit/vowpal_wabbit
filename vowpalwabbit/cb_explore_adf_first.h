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
  namespace first
  {
  LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

  struct cb_explore_adf_first : public cb_explore_adf_base
  {
   private:
    size_t m_tau;
    float m_epsilon;

   public:
    cb_explore_adf_first(size_t tau, float epsilon);
    template <bool is_learn>
    static void predict_or_learn(cb_explore_adf_first& data, LEARNER::multi_learner& base, multi_ex& examples);
    ~cb_explore_adf_first() = default;

   private:
    template <bool is_learn>
    void predict_or_learn_impl(LEARNER::multi_learner& base, multi_ex& examples);
  };
  }  // namespace first
  }  // namespace cb_explore_adf
}  // namespace VW
