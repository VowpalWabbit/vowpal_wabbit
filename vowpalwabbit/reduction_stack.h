#include "global_data.h"
#include "learner.h"
//#include "options_boost_po.h"
#include "reductions_fwd.h"

namespace VW {
namespace reduction_stack {
  VW::LEARNER::base_learner* pop_reduction(vw& all);
  void push_reduction(vw& all, VW::LEARNER::base_learner*);

  VW::LEARNER::base_learner* noop_single_setup(VW::config::options_i& options, vw& all);
  VW::LEARNER::base_learner* noop_multi_setup(VW::config::options_i& options, vw& all);
  VW::LEARNER::base_learner* passthru_single_setup(VW::config::options_i& options, vw& all);
  VW::LEARNER::base_learner* passthru_multi_setup(VW::config::options_i& options, vw& all);   
}
}
