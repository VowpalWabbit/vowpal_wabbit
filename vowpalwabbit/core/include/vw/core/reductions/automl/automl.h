// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

using namespace VW::config;
using namespace VW::LEARNER;

namespace VW
{
namespace reductions
{
VW::LEARNER::base_learner* automl_setup(VW::setup_base_i&);
}  // namespace reductions
}  // namespace VW
