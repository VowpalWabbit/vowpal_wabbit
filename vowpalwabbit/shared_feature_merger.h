/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include "learner.h"
#include "options.h"

struct vw;

namespace VW
{
namespace shared_feature_merger
{
LEARNER::base_learner* shared_feature_merger_setup(config::options_i& options, vw& all);

}  // namespace shared_feature_merger
}  // namespace VW
