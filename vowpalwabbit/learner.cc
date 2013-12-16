#include "global_data.h"
#include "parser.h"
#include "learner.h"

void save_predictor(vw& all, string reg_name, size_t current_pass);

namespace LEARNER
{
  void generic_driver(vw* all)
  {
    example* ec = NULL;

    all->l->init_driver();
    while ( true )
      {
	if ((ec = VW::get_example(all->p)) != NULL)//semiblocking operation.
	  {
	    if (ec->indices.size() > 1) // one nonconstant feature.
	      {
		all->l->learn(ec);
		all->l->finish_example(*all, ec);
	      }
	    else if (ec->end_pass)
	      {
		all->l->end_pass();
		VW::finish_example(*all,ec);
	      }
	    else if (ec->tag.size() >= 4 && !strncmp((const char*) ec->tag.begin, "save", 4))
	      {//save state

		string final_regressor_name = all->final_regressor_name;
		
		if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
		  final_regressor_name = string(ec->tag.begin+5, (ec->tag).size()-5);
		
		if (!all->quiet)
		  cerr << "saving regressor to " << final_regressor_name << endl;
		save_predictor(*all, final_regressor_name, 0);
		
		VW::finish_example(*all,ec);
	      }
	    else 
	      {
		all->l->learn(ec);

		if(all->early_terminate)
		  {
		    all->p->done = true;
		    all->l->finish_example(*all, ec);
		    return;
		  }
		else
		  {
		    all->l->finish_example(*all, ec);
		  }
	      }
	  }
	else if (parser_done(all->p))
	  {
	    all->l->end_examples();
	    return;
	  }
      }
  }
}
