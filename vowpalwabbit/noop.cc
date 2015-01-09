/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "reductions.h"

namespace NOOP {
  void learn(char&, LEARNER::base_learner&, example&) {}
  
  LEARNER::base_learner* setup(vw& all)
  {
    new_options(all, "Noop options") ("noop","do no learning");
    if(no_new_options(all)) return NULL;
    
    return &LEARNER::init_learner<char>(NULL, learn, 1); }
}
