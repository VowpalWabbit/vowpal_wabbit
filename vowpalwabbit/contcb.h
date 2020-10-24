// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "reductions.h"

namespace VW
{
namespace continuous_cb
{
VW::LEARNER::base_learner* setup(VW::config::options_i& options, vw& all);

}  // namespace continuous_cb
}  // namespace VW
