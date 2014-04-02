#ifndef SCORER_H
#define SCORER_H

namespace Scorer {
  LEARNER::learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file);
}
#endif
