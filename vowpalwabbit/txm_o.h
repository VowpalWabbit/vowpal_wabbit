/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef TXM_O_H
#define TXM_O_H

#include "oaa.h"
#include "parse_args.h"

#define TXM_O_PRED_LIM					1
#define TXM_O_PRED_ALFA   				1
#define TXM_O_LEAF_TH					0.95
#define TXM_O_LEVEL_LIM					4
#define TXM_O_LEAF_TOL					4
#define TXM_O_MULTIPLICATIVE_FACTOR		10

//#define TXM_O_OAA_MULTI

//#define TXM_O_TRAIN_ALT_TRK

//#define TXM_O_DEBUG_FILE1
//#define TXM_O_DEBUG_FILE2
//#define TXM_O_DEBUG
//#define TXM_O_DEBUG_PRED

namespace TXM_O
{  
  learner* setup(vw& all, std::vector<std::string>&, po::variables_map& vm, po::variables_map& vm_file);
  void txm_o_save_load_regressor(vw& all, io_buf& model_file, bool read, bool text);
  void txm_o_save_load(void* data, io_buf& model_file, bool read, bool text);
}

#endif
