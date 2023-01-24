// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/feature_group.h"
#include "vw/core/vw_fwd.h"

#include <memory>

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> lda_setup(VW::setup_base_i& stack_builder);
namespace lda
{
void get_top_weights(VW::workspace* all, int top_words_count, int topic, std::vector<feature>& output);
}
}  // namespace reductions
}  // namespace VW