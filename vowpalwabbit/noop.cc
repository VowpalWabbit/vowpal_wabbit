/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
// This is a learner which does nothing with examples.  Used when VW is used as a compressor.

#include "reductions.h"

using namespace LEARNER;

namespace NOOP {
  learner* setup(vw& all, po::variables_map& vm)
  {
    po::options_description opts("Noop options");
    opts.add_options()
      ("noop","do no learning");
    add_options(all, opts);
    if(!vm.count("noop"))
      return NULL;

    return new learner();
  }
}
