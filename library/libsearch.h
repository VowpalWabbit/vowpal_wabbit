#/*
COpyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#ifndef LIBSEARCH_HOOKTASK_H
#define LIBSEARCH_HOOKTASK_H

#  include "vw/core/parse_example.h"
#  include "vw/core/parser.h"
#  include "vw/core/reductions/search/search.h"
#  include "vw/core/reductions/search/search_hooktask.h"
#  include "vw/core/vw.h"

#  include <memory>

template <class INPUT, class OUTPUT>
class SearchTask
{
public:
  SearchTask(VW::workspace& vw_obj) : vw_obj(vw_obj), sch(*(Search::search*)vw_obj.searchstr)
  {
    bogus_example = VW::alloc_examples(1);
    VW::read_line(vw_obj, bogus_example, (char*)"1 | x");
    VW::setup_example(vw_obj, bogus_example);

    trigger.push_back(bogus_example);

    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    d->run_f = _search_run_fn;
    d->run_setup_f = _search_setup_fn;
    d->run_takedown_f = _search_takedown_fn;
    d->run_object = std::shared_ptr<SearchTask<INPUT, OUTPUT>>(this);
  }
  virtual ~SearchTask()
  { trigger.clear(); // the individual examples get cleaned up below
    VW::dealloc_examples(bogus_example, 1);
  }

  virtual void _run(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // YOU MUST DEFINE THIS FUNCTION!
  void       _setup(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // OPTIONAL
  void    _takedown(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // OPTIONAL

  void   learn(INPUT& input_example, OUTPUT& output) { bogus_example->test_only = false; call_vw(input_example, output); }
  void predict(INPUT& input_example, OUTPUT& output) { bogus_example->test_only = true;  call_vw(input_example, output); }

protected:
  VW::workspace& vw_obj;
  Search::search& sch;

private:
  VW::example* bogus_example;
  VW::multi_ex trigger;
  INPUT _input;
  OUTPUT _output;

  void call_vw(INPUT& input_example, OUTPUT& output)
  {
    _input = input_example;
    _output = output;
    vw_obj.learn(trigger); // this will cause our search_run_fn hook to get called
  }

  static void _search_run_fn(Search::search&sch)
  { HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if (d->run_object == nullptr) { THROW("error: calling _search_run_fn without setting run object"); }
    auto* run_obj = static_cast<SearchTask<INPUT, OUTPUT>*>(d->run_object.get());
    run_obj->_run(sch, run_obj->_input, run_obj->_output);
  }

  static void _search_setup_fn(Search::search&sch)
  { HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if (d->run_object == nullptr) { THROW("error: calling _search_setup_fn without setting run object"); }
    auto* run_obj = static_cast<SearchTask<INPUT, OUTPUT>*>(d->run_object.get());
    run_obj->_setup(sch, run_obj->_input, run_obj->_output);
  }

  static void _search_takedown_fn(Search::search&sch)
  { HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if (d->run_object == nullptr) { THROW("error: calling _search_takedown_fn without setting run object"); }
    auto* run_obj = static_cast<SearchTask<INPUT, OUTPUT>*>(d->run_object.get());
    run_obj->_takedown(sch, run_obj->_input, run_obj->_output);
  }
};

class BuiltInTask : public SearchTask<VW::multi_ex, std::vector<uint32_t>>
{
public:
  BuiltInTask(VW::workspace& vw_obj, Search::search_task* task)
      : SearchTask<VW::multi_ex, std::vector<uint32_t>>(vw_obj)
  { HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    size_t num_actions = d->num_actions;
    my_task = task;
    if (my_task->initialize)
      my_task->initialize(sch, num_actions, *d->arg);
  }

  ~BuiltInTask() { if (my_task->finish) my_task->finish(sch); }

  void _run(Search::search& sch, VW::multi_ex& input_example, std::vector<uint32_t>& output)
  { my_task->run(sch, input_example);
    sch.get_test_action_sequence(output);
  }

protected:
  Search::search_task* my_task;
};


#endif
