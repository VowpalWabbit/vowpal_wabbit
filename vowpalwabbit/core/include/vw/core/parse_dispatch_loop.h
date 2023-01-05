// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/parser.h"
#include "vw/core/v_array.h"
#include "vw/io/logger.h"

#include <functional>

namespace VW
{
namespace details
{
// DispatchFuncT should be of the form - void(VW::workspace&, const VW::multi_ex&)
template <typename DispatchFuncT>
void parse_dispatch(VW::workspace& all, DispatchFuncT& dispatch)
{
  VW::multi_ex examples;
  size_t example_number = 0;  // for variable-size batch learning algorithms

  try
  {
    while (!all.example_parser->done)
    {
      examples.push_back(&VW::get_unused_example(&all));  // need at least 1 example
      if (!all.do_reset_source && example_number != all.pass_length && all.max_examples > example_number &&
          all.example_parser->reader(&all, all.example_parser->input, examples) > 0)
      {
        VW::setup_examples(all, examples);
        example_number += examples.size();
        dispatch(all, examples);
      }
      else
      {
        VW::details::reset_source(all, all.num_bits);
        all.do_reset_source = false;
        all.passes_complete++;

        // setup an end_pass example
        all.example_parser->lbl_parser.default_label(examples[0]->l);
        examples[0]->end_pass = true;
        all.example_parser->in_pass_counter = 0;
        // Since this example gets finished, we need to keep the counter correct.
        all.example_parser->num_setup_examples++;

        if (all.passes_complete == all.numpasses && example_number == all.pass_length)
        {
          all.passes_complete = 0;
          all.pass_length = all.pass_length * 2 + 1;
        }
        dispatch(all, examples);  // must be called before lock_done or race condition exists.
        if (all.passes_complete >= all.numpasses && all.max_examples >= example_number)
        {
          VW::details::lock_done(*all.example_parser);
        }
        example_number = 0;
      }

      examples.clear();
    }
  }
  catch (VW::vw_exception& e)
  {
    VW::return_multiple_example(all, examples);
    all.logger.err_error("vw example #{0}({1}:{2}): {3}", example_number, e.filename(), e.line_number(), e.what());

    // Stash the exception so it can be thrown on the main thread.
    all.example_parser->exc_ptr = std::current_exception();
  }
  catch (std::exception& e)
  {
    VW::return_multiple_example(all, examples);
    all.logger.err_error("vw: example #{0}{1}", example_number, e.what());

    // Stash the exception so it can be thrown on the main thread.
    all.example_parser->exc_ptr = std::current_exception();
  }
  VW::details::lock_done(*all.example_parser);
}

}  // namespace details
}  // namespace VW
