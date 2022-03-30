// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "io/logger.h"
#include "vw_fwd.h"

#include <vector>

namespace CB
{
struct cb_class;
}

namespace VW
{
namespace io
{
struct writer;
}
}  // namespace VW

VW::LEARNER::base_learner* cb_adf_setup(VW::setup_base_i& stack_builder);

namespace CB_ADF
{
void global_print_newline(
    const std::vector<std::unique_ptr<VW::io::writer>>& final_prediction_sink, VW::io::logger& logger);
VW::example* test_adf_sequence(const VW::multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const VW::multi_ex& examples);
}  // namespace CB_ADF
