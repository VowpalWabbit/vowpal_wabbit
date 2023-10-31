// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"
#include <memory>

namespace VW
{
namespace reductions
{
class dnn_learner
{
public:
  void init();
  void predict(example& ec);
  void learn(const example& ec);
};

std::shared_ptr<VW::LEARNER::learner> dnn_setup(VW::setup_base_i& stack_builder);
}  // namespace reductions
}  // namespace VW
