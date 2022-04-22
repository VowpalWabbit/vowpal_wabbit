// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/config/options.h"
#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/learner.h"

namespace VW
{
namespace reductions
{
LEARNER::base_learner* pmf_to_pdf_setup(VW::setup_base_i& stack_builder);
struct pmf_to_pdf_reduction
{
  void predict(example& ec);
  void learn(example& ec);

  std::vector<float> pdf_lim;
  uint32_t num_actions = 0;
  uint32_t tree_bandwidth = 0;
  float bandwidth = 0.f;  // radius
  float min_value = 0.f;
  float max_value = 0.f;
  bool first_only = false;
  LEARNER::single_learner* _p_base = nullptr;

private:
  void transform_prediction(example& ec);

  CB::label temp_lbl_cb;
  ACTION_SCORE::action_scores temp_pred_a_s;
};
}  // namespace reductions
}  // namespace VW
