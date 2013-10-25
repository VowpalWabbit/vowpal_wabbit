#include "global_data.h"
#include "parser.h"
#include "learner.h"

namespace LEARNER
{
  void driver(vw* all, void* data)
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
