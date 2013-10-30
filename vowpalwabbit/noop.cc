/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "vw.h"

namespace NOOP {
  void learn(void* d, example*ec) {}

  learner setup(vw& all)
  {
    learner l(NULL,learn);
    return l;
  }
}
