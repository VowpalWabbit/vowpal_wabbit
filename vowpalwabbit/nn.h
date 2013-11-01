/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef NN_H
#define NN_H

#include "global_data.h"
#include "parse_args.h"

namespace NN
{
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
