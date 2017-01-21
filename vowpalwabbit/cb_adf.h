#pragma once

LEARNER::base_learner* cb_adf_setup(vw& all);

namespace CB_ADF
{
CB::cb_class get_observed_cost(v_array<example*>& examples);
void global_print_newline(vw& all);
}
