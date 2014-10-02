/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEARN_DEP_PARSER_H
#define SEARN_DEP_PARSER_H

#include "search.h"

namespace DepParserTask {
  void initialize(Search::search&, size_t&, po::variables_map&);
  void finish(Search::search&);
  void run(Search::search&, vector<example*>&);
  extern Search::search_task task;
}

#endif
