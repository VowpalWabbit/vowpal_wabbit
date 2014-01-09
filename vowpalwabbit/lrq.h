
#ifndef LRQ_HEADER
#define LRQ_HEADER
namespace LRQ {
  LEARNER::learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file);
}
#endif
