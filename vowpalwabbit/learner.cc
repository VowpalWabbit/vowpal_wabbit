#include "global_data.h"
#include "parser.h"
#include "learner.h"

namespace LEARNER
{
  void generic_sl(void*, io_buf&, bool, bool)
  { cout << "calling generic_save_load";}
  void generic_learner(void* data, example*)
  { cout << "calling generic learner\n";}
  void generic_end_pass(void* data)
  { cout << "calling generic end_pass\n";}
  void generic_end_examples(void* data)
  { cout << "calling generic end_examples\n";}
  void generic_finish(void* data)
  { cout << "calling generic finish\n";}

  void generic_driver(vw* all, void* data)
  {
    example* ec = NULL;
    
    while ( true )
      {
	if(all->early_terminate)
	  {
	    all->p->done = true;
	    return;
	  }
	else if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
	  {
	    all->l.learn(ec);
	    return_simple_example(*all, ec);
	  }
	else if (parser_done(all->p))
	  return;
	else 
	  ;//busywait when we have predicted on all examples but not yet trained on all.
      }
  }
}
