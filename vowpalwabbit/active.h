#pragma once
namespace ACTIVE {
  po::options_description options();
  LEARNER::learner* setup(vw& all, po::variables_map& vm);
}
