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

  void drive(vw* all, void* d)
  {
    example* ec = NULL;
    while ( true )
      {
        if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn(d, ec);
	    OAA::output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
	  return;
        else 
          ;
      }
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
    learner l = {data, drive, learn, finish, all.l.sl};
    return l;
  }
}
