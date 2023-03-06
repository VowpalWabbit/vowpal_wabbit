// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/action_score.h"
#include "vw/core/cb.h"
#include "vw/core/learner_fwd.h"
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> pmf_to_pdf_setup(VW::setup_base_i& stack_builder);
class pmf_to_pdf_reduction
{
public:
  void predict(example& ec);
  void learn(example& ec);

  std::vector<float> pdf_lim;
  uint32_t num_actions = 0;
  uint32_t tree_bandwidth = 0;
  float bandwidth = 0.f;  // radius
  float min_value = 0.f;
  float max_value = 0.f;
  bool first_only = false;
  LEARNER::learner* _p_base = nullptr;

private:
  void transform_prediction(example& ec);

  VW::cb_label temp_lbl_cb;
  VW::action_scores temp_pred_a_s;
};
}  // namespace reductions
}  // namespace VW
