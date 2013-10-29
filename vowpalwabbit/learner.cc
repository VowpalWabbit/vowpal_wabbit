#include "global_data.h"
#include "parser.h"
#include "learner.h"

namespace LEARNER
{
  void generic_driver(vw* all, void* data)
  {
    example* ec = NULL;

    all->l.init_driver();
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
	    all->l.finish_example(*all, ec);
	  }
	else if (parser_done(all->p))
	  {
	    all->l.end_examples();
	    return;
	  }
      }
  }
}
