#pragma once

// TODO: this circular dependency would be a problem, but 
// it disappears when we move into real VW land
#include "vwtypes.h"

namespace pseudo_vw
{
  struct stack_builder_t
  {
    //void delayed_state_attach()

    VW::LEARNER::base_learner* setup_base_learner();
    VW::config::options_i* get_options();
    magic_t* get_all_pointer();
  };

  namespace VW
  {
    using setup_base_i = ::pseudo_vw::stack_builder_t;
  }
}