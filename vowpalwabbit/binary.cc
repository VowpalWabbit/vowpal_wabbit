#include "oaa.h"

namespace BINARY {
  struct binary {
    learner base;
  };

  void learn(void*a, void* d, example* ec)
  {
    binary* b = (binary*)d;
    b->base.learn(a, b->base.data, ec);
    
    float prediction = -1;
    if ( ec->final_prediction > 0)
      prediction = 1;
    ec->final_prediction = prediction;
  }

  void finish(void*a, void* d)
  {
    binary* b = (binary*)d;
    b->base.finish(a,b->base.data);
    free(b);
  }

  void drive(void *in, void* d)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn(all, d, ec);
	    OAA::output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
	  return;
        else 
          ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
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
    learner l = {data, drive, learn, finish, all.l.save_load};
    all.l = l;
  }
}
