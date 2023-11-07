// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/reductions/search/search_hooktask.h"

#include "vw/io/logger.h"

using namespace VW::config;

// this is used for the C++ library and python library hook; hopefully
// it can be used for any foreign library too!
namespace HookTask
{
Search::search_task task = {"hook", run, initialize, nullptr, run_setup, run_takedown};

void initialize(Search::search& sch, size_t& num_actions, options_i& arg)
{
  task_data* td = new task_data;
  td->run_f = nullptr;
  td->run_setup_f = nullptr;
  td->run_takedown_f = nullptr;
  td->run_object = nullptr;
  td->setup_object = nullptr;
  td->takedown_object = nullptr;
  td->num_actions = num_actions;
  td->arg = &arg;
  sch.set_task_data<task_data>(td);
}

void run(Search::search& sch, VW::multi_ex& /*ec*/)
{
  task_data* td = sch.get_task_data<task_data>();
  if (td->run_f) { td->run_f(sch); }
  else { sch.get_vw_pointer_unsafe().logger.err_warn("HookTask::structured_predict called before hook is set"); }
}

void run_setup(Search::search& sch, VW::multi_ex& /*ec*/)
{
  task_data* td = sch.get_task_data<task_data>();
  if (td->run_setup_f) { td->run_setup_f(sch); }
}

void run_takedown(Search::search& sch, VW::multi_ex& /*ec*/)
{
  task_data* td = sch.get_task_data<task_data>();
  if (td->run_takedown_f) { td->run_takedown_f(sch); }
}
}  // namespace HookTask
