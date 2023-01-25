#pragma once

#include "../base.h"
#include "learner_types.h"

namespace pseudo_vw
{
  struct shared_data{
    float min_label = 0.f;  // minimum label encountered
    float max_label = 0.f;  // maximum label encountered
  };
  
  class workspace{
    public:
      shared_data* sd;

      VW::LEARNER::base_learner* l;  // the top level learner
  };

  namespace VW {
    using workspace = pseudo_vw::workspace;
  }
}