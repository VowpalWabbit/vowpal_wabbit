#pragma once

LEARNER::base_learner* cb_adf_setup(arguments& arg);

namespace CB_ADF
{
CB::cb_class get_observed_cost(multi_ex& examples);
void global_print_newline(vw& all);
}
