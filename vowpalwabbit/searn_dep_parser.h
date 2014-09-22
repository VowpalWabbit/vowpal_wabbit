/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEARN_DEP_PARSER_H
#define SEARN_DEP_PARSER_H

#include "searn.h"

namespace DepParserTask {
  void initialize(Searn::searn&, size_t&, po::variables_map&);
  void finish(Searn::searn&);
  void structured_predict(Searn::searn&, vector<example*>&);
  extern Searn::searn_task task;
}

#endif
