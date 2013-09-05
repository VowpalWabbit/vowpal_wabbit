/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef TXM_O_H
#define TXM_O_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"
#include "v_hashmap.h"

#define TXM_O_PRED_LIM	1
#define TXM_O_PRED_ALFA   1

#define TXM_O_DEBUG
#define TXM_O_DEBUG_EX_STOP
//#define TXM_O_DEBUG_PRED

namespace TXM_O
{  
  learner setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
}

#endif
