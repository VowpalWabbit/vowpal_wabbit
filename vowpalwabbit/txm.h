/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef TXM_H
#define TXM_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "parse_args.h"
#include "v_hashmap.h"

#define TXM_PRED_LIM	1.f
#define TXM_PRED_ALFA   1.f
#define TXM_LEAF_TH		0.95
#define TXM_LEVEL_LIM	5
#define TXM_MULTIPLICATIVE_FACTOR	10

#define TXM_DEBUG_FILE1
#define TXM_DEBUG_FILE2
//#define TXM_DEBUG
//#define TXM_DEBUG_PASS_STOP
//#define TXM_DEBUG_EX_STOP
//#define TXM_DEBUG_PRED

namespace TXM
{
  learner setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
  void txm_save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
  void txm_save_load(void* data, io_buf& model_file, bool read, bool text);
}

#endif
