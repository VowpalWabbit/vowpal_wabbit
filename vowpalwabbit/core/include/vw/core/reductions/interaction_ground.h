// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/reduction_stack.h"
#include "vw/core/reductions/ftrl.h"

#include <memory>

namespace VW
{
namespace reductions
{
/**
 * Setup interaction grounded learning reduction. Wiki page:
 * https://github.com/VowpalWabbit/vowpal_wabbit/wiki/Interaction-Grounded-Learning
 *
 * @param stack_builder Stack builder to use for setup.
 * @return VW::LEARNER::learner* learner if this reduction is active, nullptr otherwise
 */
std::shared_ptr<VW::LEARNER::learner> interaction_ground_setup(VW::setup_base_i& stack_builder);

namespace igl
{
class igl_data
{
public:
  igl_data(bool predict_only_model);

  std::shared_ptr<VW::LEARNER::learner> ik_learner = nullptr;
  VW::example ik_ex;

  VW::cb_class known_cost;  // for update stats

  std::vector<std::vector<VW::namespace_index>> interactions;
  std::vector<std::vector<VW::extent_term>>* extent_interactions;

  std::unique_ptr<VW::workspace> ik_all;
  ftrl* ik_ftrl;  // automatically save resume
  std::unique_ptr<ftrl> pi_ftrl;

  bool _predict_only_model;
  size_t _cb_model_offset = 1;
  size_t _feature_width = 2;
};
}  // namespace igl
}  // namespace reductions

namespace model_utils
{
size_t write_model_field(io_buf&, const VW::reductions::igl::igl_data&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::reductions::igl::igl_data&);
}  // namespace model_utils
}  // namespace VW
