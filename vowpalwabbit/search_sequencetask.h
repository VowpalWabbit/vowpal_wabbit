/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "search.h"

namespace SequenceTask {
  void initialize(Search::search&, size_t&, po::variables_map&);
  void run(Search::search&, vector<example*>&);
  extern Search::search_task task;
}

namespace SequenceSpanTask {
  void initialize(Search::search&, size_t&, po::variables_map&);
  void finish(Search::search&);
  void run(Search::search&, vector<example*>&);
  void setup(Search::search&, vector<example*>&);
  void takedown(Search::search&, vector<example*>&);
  extern Search::search_task task;
}

namespace ArgmaxTask {
  void initialize(Search::search&, size_t&, po::variables_map&);
  void run(Search::search&, vector<example*>&);
  extern Search::search_task task;
}

namespace SequenceTask_DemoLDF {
  void initialize(Search::search&, size_t&, po::variables_map&);
  void finish(Search::search&);
  void run(Search::search&, vector<example*>&);
  extern Search::search_task task;
}
