
#ifndef AUTOLINK_H
#define AUTOLINK_H
namespace ALINK {
  LEARNER::learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file);
}
#endif
