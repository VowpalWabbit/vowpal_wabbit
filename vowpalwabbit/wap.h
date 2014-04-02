/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef WEIGHTED_ALL_PAIRS_H
#define WEIGHTED_ALL_PAIRS_H

#include "global_data.h"
#include "parse_args.h"

namespace WAP {
  LEARNER::learner* setup(vw&, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
