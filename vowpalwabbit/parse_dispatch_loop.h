// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <functional>
#include <thread>
#include <future>
#include <mutex>  
#include "io_to_queue.h"

#include "io/logger.h"

using dispatch_fptr = std::function<void(vw&, const v_array<example*>&)>;
struct io_state;

inline void parse_dispatch(vw& all, dispatch_fptr dispatch)
{
  v_array<example*> examples;
  size_t example_number = 0;  // for variable-size batch learning algorithms

  // for substring_to_example
  std::vector<VW::string_view> words_localcpy;
  std::vector<VW::string_view> parse_name_localcpy;

  try
  {
    while (!all.example_parser->done)
    {

      example* example_ptr = &VW::get_unused_example(&all);
      examples.push_back(example_ptr);

      if (!all.do_reset_source && example_number != all.pass_length && all.max_examples > example_number &&
          all.example_parser->reader(&all, examples, words_localcpy, parse_name_localcpy) > 0)
      {

        VW::setup_examples(all, examples);
        example_number += examples.size();

        dispatch(all, examples);

      }
      else
      { 

        reset_source(all, all.num_bits);

        // to call reset source in io thread
        all.example_parser->done_with_io.store(true);
        all.example_parser->can_end_pass.notify_one();

        all.do_reset_source = false;
        all.passes_complete++;

        // setup an end_pass example
        all.example_parser->lbl_parser.default_label(&examples[0]->l);
        examples[0]->end_pass = true;
        all.example_parser->in_pass_counter = 0;

        if (all.passes_complete == all.numpasses && example_number == all.pass_length)
        {
          all.passes_complete = 0;
          all.pass_length = all.pass_length * 2 + 1;
        }

        dispatch(all, examples);  // must be called before lock_done or race condition exists.
        if (all.passes_complete >= all.numpasses && all.max_examples >= example_number) lock_done(*all.example_parser);
        example_number = 0;

      }

      examples.clear();
    }

  }
  
  catch (VW::vw_exception& e)
  {
    VW::io::logger::errlog_error("vw example #{0}({1}:{2}): {3}", example_number, e.Filename(), e.LineNumber(), e.what());

    // Stash the exception so it can be thrown on the main thread.
    all.example_parser->exc_ptr = std::current_exception();
  }
  catch (std::exception& e)
  {
    VW::io::logger::errlog_error("vw: example #{0}{1}", example_number, e.what());

    // Stash the exception so it can be thrown on the main thread.
    all.example_parser->exc_ptr = std::current_exception();
  }
  lock_done(*all.example_parser);
}
