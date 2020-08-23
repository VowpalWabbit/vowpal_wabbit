// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <functional>
#include <thread>
#include <future>
#include <mutex>  
#include "io_to_queue.h"

using dispatch_fptr = std::function<void(vw&, const v_array<example*>&)>;
struct io_state;

inline void parse_dispatch(vw& all, dispatch_fptr dispatch)
{

  v_array<example*> examples = v_init<example*>();
  size_t example_number = 0;  // for variable-size batch learning algorithms

  // for substring_to_example
  v_array<VW::string_view> words_localcpy = v_init<VW::string_view>();
  v_array<VW::string_view> parse_name_localcpy = v_init<VW::string_view>();

  try
  {

    while (!all.p->done)
    {

      example* example_ptr = &VW::get_unused_example(&all);
      examples.push_back(example_ptr);

      if (!all.do_reset_source && example_number != all.pass_length && all.max_examples > example_number &&
          all.p->reader(&all, examples, words_localcpy, parse_name_localcpy) > 0)
      {

        VW::setup_examples(all, examples);
        example_number += examples.size();

        dispatch(all, examples);

      }
      else
      { 

        reset_source(all, all.num_bits);

        // to call reset source in io thread
        all.p->done_with_io.store(true);
        all.p->can_end_pass.notify_one();

        all.do_reset_source = false;
        all.passes_complete++;

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
