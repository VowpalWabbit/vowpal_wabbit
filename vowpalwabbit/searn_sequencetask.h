/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEARN_SEQUENCETASK_H
#define SEARN_SEQUENCETASK_H

#include "oaa.h"
#include "parse_primitives.h"
#include "searn.h"

namespace SequenceTask {
  void initialize(vw&, Searn::searn&, size_t&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void finish(vw&, Searn::searn&);
  void structured_predict_v1(vw&, Searn::searn&, example**,size_t,stringstream*,stringstream*);
}


#endif
