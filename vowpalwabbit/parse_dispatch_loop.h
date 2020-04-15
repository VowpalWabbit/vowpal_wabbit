// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <functional>

using dispatch_fptr = std::function<void(vw&, const v_array<example*>&)>;

inline void parse_dispatch(vw& all, dispatch_fptr dispatch)
{
  v_array<example*> examples = v_init<example*>();
  size_t example_number = 0;  // for variable-size batch learning algorithms

  try
  {
    while (!all.p->done)
    {
      examples.push_back(&VW::get_unused_example(&all));  // need at least 1 example
      if (!all.do_reset_source && example_number != all.pass_length && all.max_examples > example_number &&
          all.p->reader(&all, examples) > 0)
      {
        VW::setup_examples(all, examples);
        example_number += examples.size();
        dispatch(all, examples);
      }
      else
      {
        reset_source(all, all.num_bits);
        all.do_reset_source = false;
        all.passes_complete++;

        // setup an end_pass example
        all.p->lp.default_label(&examples[0]->l);
        examples[0]->end_pass = true;
        all.p->in_pass_counter = 0;

        if (all.passes_complete == all.numpasses && example_number == all.pass_length)
        {
          all.passes_complete = 0;
          all.pass_length = all.pass_length * 2 + 1;
        }
        dispatch(all, examples);  // must be called before lock_done or race condition exists.
        if (all.passes_complete >= all.numpasses && all.max_examples >= example_number)
          lock_done(*all.p);
        example_number = 0;
      }

      examples.clear();
    }
  }
  catch (VW::vw_exception& e)
  {
    std::cerr << "vw example #" << example_number << "(" << e.Filename() << ":" << e.LineNumber() << "): " << e.what()
              << std::endl;

    // Stash the exception so it can be thrown on the main thread.
    all.p->exc_ptr = std::current_exception();
  }
  catch (std::exception& e)
  {
    std::cerr << "vw: example #" << example_number << e.what() << std::endl;

    // Stash the exception so it can be thrown on the main thread.
    all.p->exc_ptr = std::current_exception();
  }
  lock_done(*all.p);
  examples.delete_v();
}
