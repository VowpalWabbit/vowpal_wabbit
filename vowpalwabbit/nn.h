/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include "global_data.h"
#include "parse_args.h"

namespace NN
{
  LEARNER::learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
