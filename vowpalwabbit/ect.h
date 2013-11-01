/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef ECT_H
#define ECT_H

#include "oaa.h"
#include "parse_args.h"

namespace ECT
{
  learner* setup(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map& vm_file);
}

#endif
