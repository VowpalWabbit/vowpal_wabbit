/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
namespace CSOAA {
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
}

namespace CSOAA_AND_WAP_LDF {
  LEARNER::learner* setup(vw& all, po::variables_map& vm);

namespace LabelDict { 
  bool ec_is_example_header(example& ec);  // example headers look like "0:-1" or just "shared"
  void add_example_namespaces_from_example(example& target, example& source);
  void del_example_namespaces_from_example(example& target, example& source);
}

}
