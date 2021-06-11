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

      int num_chars_read = all.example_parser->reader(&all, examples, words_localcpy, parse_name_localcpy);
      if(num_chars_read > 0){
        dispatch(all, examples);
      }
      else if(num_chars_read == 0){
        examples[0]->end_pass = true;
        dispatch(all, examples);
      }
      else if(num_chars_read == -1)
        VW::finish_example(all, *example_ptr);
  
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
