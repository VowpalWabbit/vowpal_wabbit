// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "reductions_fwd.h"
#include "example_predict.h"

#include <vector>

VW::LEARNER::base_learner* generate_interactions_setup(VW::config::options_i& options, vw& all);


std::vector<std::vector<namespace_index>> generate_combinations_with_repetiton(
    const std::vector<namespace_index>& namespaces, size_t num_to_pick);
std::vector<std::vector<namespace_index>> generate_permutations_with_repetition(
    const std::vector<namespace_index>& namespaces, size_t num_to_pick);
