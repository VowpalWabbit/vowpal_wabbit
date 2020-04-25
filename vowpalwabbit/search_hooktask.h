// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "search.h"

namespace HookTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void finish(Search::search&);
void run(Search::search&, multi_ex&);
void run_setup(Search::search&, multi_ex&);
void run_takedown(Search::search&, multi_ex&);
extern Search::search_task task;

struct task_data
{
  void (*run_f)(Search::search&);
  void (*run_setup_f)(Search::search&);
  void (*run_takedown_f)(Search::search&);
  void* run_object;  // for python this will really be a (py::object*), but we don't want basic VW to have to know about
                     // hook
  void* setup_object;     // for python this will really be a (py::object*), but we don't want basic VW to have to know
                          // about hook
  void* takedown_object;  // for python this will really be a (py::object*), but we don't want basic VW to have to know
                          // about hook
  void (*delete_run_object)(
      void*);  // we can't delete run_object on our own because we don't know its size, so provide a hook
  void (*delete_extra_data)(task_data&);  // ditto for extra_data and extra_data2
  VW::config::options_i* arg;             // so that hook can access command line variables
  const void* extra_data;                 // any extra data that might be needed
  const void* extra_data2;                // any (more) extra data that might be needed
  size_t num_actions;                     // cache for easy access
};
}  // namespace HookTask
