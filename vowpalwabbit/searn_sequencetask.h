/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEARN_SEQUENCETASK_H
#define SEARN_SEQUENCETASK_H

#include "searn.h"

namespace SequenceTask {
  void initialize(Searn::searn&, size_t&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void finish(Searn::searn&);
  void structured_predict(Searn::searn&, example**,size_t,stringstream*,stringstream*);
}

namespace OneOfManyTask {
  void initialize(Searn::searn&, size_t&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void finish(Searn::searn&);
  void structured_predict(Searn::searn&, example**,size_t,stringstream*,stringstream*);
}

namespace SequenceSpanTask {
  void initialize(Searn::searn&, size_t&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void finish(Searn::searn&);
  void structured_predict(Searn::searn&, example**,size_t,stringstream*,stringstream*);
}

namespace SequenceTask_DemoLDF {
  void initialize(Searn::searn&, size_t&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void finish(Searn::searn&);
  void structured_predict(Searn::searn&, example**,size_t,stringstream*,stringstream*);
  void update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount);
}


#endif
