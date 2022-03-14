// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <set>
#include <vector>

#include "example_predict.h"
#include "interactions.h"
#include "reductions_fwd.h"

VW::LEARNER::base_learner* generate_interactions_setup(VW::setup_base_i& stack_builder);
