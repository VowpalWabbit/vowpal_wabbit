/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef STAGEWISE_POLY_H
#define STAGEWISE_POLY_H

namespace StagewisePoly
{
  //LEARNER::learner *setup(vw &all, std::vector<std::string> &, po::variables_map &vm, po::variables_map &vm_file);
  LEARNER::learner *setup(vw &all, po::variables_map &vm);
}

#endif
