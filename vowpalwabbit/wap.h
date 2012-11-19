/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"

namespace WAP {
  void parse_flags(vw&, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
