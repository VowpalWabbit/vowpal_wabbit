#include "oaa.h"
#include "vw.h"

namespace BINARY {
  struct binary {
    learner base;
  };

  void learn(void* d, example* ec)
  {
    binary* b = (binary*)d;
    b->base.learn(ec);
    
    float prediction = -1;
    if ( ec->final_prediction > 0)
      prediction = 1;
    ec->final_prediction = prediction;
  }

  void finish(void* d)
  {
    binary* b = (binary*)d;
    b->base.finish();
    free(b);
  }

  learner setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {
    if (!vm_file.count("binary")) 
      {
	std::stringstream ss;
	ss << " --binary ";
	all.options_from_file.append(ss.str());
      }

    all.sd->binary_label = true;
    binary* data = (binary*)calloc(1,sizeof(binary));
    data->base = all.l;
    learner l(data, LEARNER::generic_driver, learn, finish, all.l.sl);

    l.set_finish_example(OAA::finish_example);
    l.set_base(&(data->base));
    return l;
  }
}
