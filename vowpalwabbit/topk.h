/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef TOPK_H
#define TOPK_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"
#include "v_hashmap.h"
#include "simple_label.h"

namespace TOPK
{
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
