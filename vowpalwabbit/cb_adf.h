// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include <vector>
#include "reductions_fwd.h"

VW::LEARNER::base_learner* cb_adf_setup(VW::config::options_i& options, vw& all);

namespace CB_ADF
{
void global_print_newline(const std::vector<std::unique_ptr<VW::io::writer>>& final_prediction_sink);
example* test_adf_sequence(multi_ex& ec_seq);
CB::cb_class get_observed_cost_or_default_cb_adf(const multi_ex& examples);
struct cb_adf;
void update_and_output(vw& all, cb_adf& data, multi_ex& ec_seq);
}  // namespace CB_ADF
