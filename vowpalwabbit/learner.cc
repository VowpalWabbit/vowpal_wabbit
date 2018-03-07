#include "parser.h"
#include "vw.h"
#include "parse_regressor.h"
using namespace std;

void dispatch_example(vw& all, example& ec)
{
  all.learn(&ec);
  all.l->finish_example(all, ec);
}

namespace prediction_type
{
#define CASE(type) case type: return #type;

const char* to_string(prediction_type_t prediction_type)
{
  switch (prediction_type)
  {
    CASE(scalar)
    CASE(scalars)
    CASE(action_scores)
    CASE(action_probs)
    CASE(multiclass)
    CASE(multilabels)
    CASE(prob)
    CASE(multiclassprobs)
  default: return "<unsupported>";
  }
}
}

namespace LEARNER
{
void process_example(vw& all, example* ec)
{
  if (ec->indices.size() > 1) // 1+ nonconstant feature. (most common case first)
    dispatch_example(all, *ec);
  else if (ec->end_pass)
  {
    all.l->end_pass();
    VW::finish_example(all, ec);
  }
  else if (ec->tag.size() >= 4 && !strncmp((const char*) ec->tag.begin(), "save", 4))
  {
    // save state command

    string final_regressor_name = all.final_regressor_name;

    if ((ec->tag).size() >= 6 && (ec->tag)[4] == '_')
      final_regressor_name = string(ec->tag.begin()+5, (ec->tag).size()-5);

    if (!all.quiet)
      all.opts_n_args.trace_message << "saving regressor to " << final_regressor_name << endl;
    save_predictor(all, final_regressor_name, 0);

    VW::finish_example(all,ec);
  }
  else // empty example
    dispatch_example(all, *ec);
}

  template <class T, void(*f)(T, example*)> void generic_driver_onethread(vw& x_all, T context)
{
  vw* all = &x_all;

  v_array<example*> examples = v_init<example*>();
  size_t example_number = 0;  // for variable-size batch learning algorithms

  // XXX get rid of ring buffer there?

  try
  {
    size_t examples_available;
    while(!all->p->done)
    {
      examples.push_back(&VW::get_unused_example(all)); // need at least 1 example
      if (!all->do_reset_source && example_number != all->pass_length && all->max_examples > example_number && all->p->reader(all, examples) > 0)
      {
        VW::setup_examples(*all, examples);
        example_number+=examples.size();
        examples_available=examples.size();
      }
      else
      {
        reset_source(*all, all->num_bits);
        all->do_reset_source = false;
        all->passes_complete++;

        examples[0]->end_pass = true;
        all->p->in_pass_counter = 0;

        if (all->passes_complete == all->numpasses && example_number == all->pass_length)
        {
          all->passes_complete = 0;
          all->pass_length = all->pass_length*2+1;
        }
        if (all->passes_complete >= all->numpasses && all->max_examples >= example_number)
        {
          all->p->done = true;
        }
        example_number = 0;
        examples_available=1;
      }

      all->p->end_parsed_examples+=examples_available;

      for (size_t i = 0; i < examples_available; ++i)
        f(context, examples[i]);

      examples.erase();
    }
  }
  catch (VW::vw_exception& e)
  {
    cerr << "vw example #" << example_number << "(" << e.Filename() << ":" << e.LineNumber() << "): " << e.what() << endl;
  }
  catch (exception& e)
  {
    cerr << "vw: example #" << example_number << e.what() << endl;
  }

  all->p->done = true;
  examples.delete_v();
}

template <class T, void(*f)(T, example*)> void generic_driver(vw& all, T context)
{
  example* ec = nullptr;

  while ( all.early_terminate == false )
    if ((ec = VW::get_example(all.p)) != nullptr)
      f(context, ec);
    else
      break;
  if (all.early_terminate) //drain any extra examples from parser.
    while ((ec = VW::get_example(all.p)) != nullptr)
      VW::finish_example(all, ec);
  all.l->end_examples();
}

void process_multiple(vector<vw*> alls, example* ec)
{
  // start with last as the first instance will free the example as it is the owner
  for (auto it = alls.rbegin(); it != alls.rend(); ++it)
    process_example(**it, ec);
}

void generic_driver(vector<vw*> alls)
{
  generic_driver<vector<vw*>, process_multiple>(**alls.begin(), alls);

  // skip first as it already called end_examples()
  auto it = alls.begin();
  for (it++; it != alls.end(); it++)
    (*it)->l->end_examples();
}

void generic_driver(vw& all)
{ generic_driver<vw&, process_example>(all, all); }

  void set_done(parser& p){p.done=true;}

void generic_driver_onethread(vw& all)
{
  generic_driver_onethread<vw&, process_example>(all, all);
  all.l->end_examples();
}
}
