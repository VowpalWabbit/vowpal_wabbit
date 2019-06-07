#pragma once

LEARNER::base_learner* cb_adf_setup(VW::config::options_i& options, vw& all);

namespace CB_ADF
{
CB::cb_class get_observed_cost(multi_ex& examples);
void global_print_newline(vw& all);
}  // namespace CB_ADF
