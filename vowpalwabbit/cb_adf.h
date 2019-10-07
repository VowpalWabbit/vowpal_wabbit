#pragma once
#include <vector>
#include "reductions_fwd.h"

LEARNER::base_learner* cb_adf_setup(VW::config::options_i& options, vw& all);

namespace CB_ADF
{
CB::cb_class get_observed_cost(multi_ex& examples);
void global_print_newline(const v_array<int>& final_prediction_sink);
example* test_adf_sequence(multi_ex& ec_seq);
}  // namespace CB_ADF
