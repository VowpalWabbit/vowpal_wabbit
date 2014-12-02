/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "global_data.h"
#include "parse_args.h"

namespace NN
{
  po::options_description options();
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
}
