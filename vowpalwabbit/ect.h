/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef ERROR_CORRECTING_H
#define ERROR_CORRECTING_H

namespace ECT
{
  LEARNER::learner* setup(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map& vm_file);
}

#endif
