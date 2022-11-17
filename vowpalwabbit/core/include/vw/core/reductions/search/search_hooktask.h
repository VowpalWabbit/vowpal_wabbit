// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "search.h"

namespace HookTask
{
void initialize(Search::search&, size_t&, VW::config::options_i&);
void run(Search::search&, VW::multi_ex&);
void run_setup(Search::search&, VW::multi_ex&);
void run_takedown(Search::search&, VW::multi_ex&);
extern Search::search_task task;

class task_data
{
public:
  void (*run_f)(Search::search&) = nullptr;
  void (*run_setup_f)(Search::search&) = nullptr;
  void (*run_takedown_f)(Search::search&) = nullptr;
  std::shared_ptr<void> run_object;    // for python this will really be a (py::object*), but we don't want basic VW to
                                       // have to know about hook
  std::shared_ptr<void> setup_object;  // for python this will really be a (py::object*), but we don't want basic VW to
                                       // have to know about hook
  std::shared_ptr<void> takedown_object;  // for python this will really be a (py::object*), but we don't want basic VW
                                          // to have to know about hook
  VW::config::options_i* arg = nullptr;   // so that hook can access command line variables
  size_t num_actions = 0;                 // cache for easy access
};
}  // namespace HookTask
