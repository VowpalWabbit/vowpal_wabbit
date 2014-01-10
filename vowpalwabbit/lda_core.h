/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LDA_CORE_H
#define LDA_CORE_H

namespace LDA{
  LEARNER::learner* setup(vw&, std::vector<std::string>&, po::variables_map&);
}

#endif
