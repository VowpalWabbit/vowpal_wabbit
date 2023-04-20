// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "vw/core/reductions/gd.h"
#include "vw/core/vw_fwd.h"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace VW
{
namespace reductions
{
class ftrl_update_data
{
public:
  float update = 0.f;
  float ftrl_alpha = 0.f;
  float ftrl_beta = 0.f;
  float l1_lambda = 0.f;
  float l2_lambda = 0.f;
  float predict = 0.f;
  float normalized_squared_norm_x = 0.f;
  float average_squared_norm_x = 0.f;
};

class ftrl
{
public:
  VW::workspace* all = nullptr;  // features, finalize, l1, l2,
  float ftrl_alpha = 0.f;
  float ftrl_beta = 0.f;
  ftrl_update_data data;
  size_t no_win_counter = 0;
  size_t early_stop_thres = 0;
  uint32_t ftrl_size = 0;
  std::vector<VW::reductions::details::gd_per_model_state> gd_per_model_states;
};

namespace model_utils
{
size_t read_model_field(io_buf&, VW::reductions::ftrl&);
size_t write_model_field(io_buf&, VW::reductions::ftrl&, const std::string&, bool);
}  // namespace model_utils

std::shared_ptr<VW::LEARNER::learner> ftrl_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW
