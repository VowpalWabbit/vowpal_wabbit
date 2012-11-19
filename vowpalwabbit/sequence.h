/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"

namespace Sequence {
  void parse_flags(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void drive(void*);
}
#endif
