/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef BFGS_H
#define BFGS_H
#include "gd.h"

namespace BFGS {
  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
