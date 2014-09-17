#/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LIBSEARN_HOOKTASK_H
#define LIBSEARN_HOOKTASK_H

#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/searn.h"
#include "../vowpalwabbit/searn_hooktask.h"

using namespace std;

template<class INPUT, class OUTPUT> class SearchTask {
  public:
  SearchTask(vw& vw_obj) : vw_obj(vw_obj), srn(*(Searn::searn*)vw_obj.searnstr) {
    bogus_example = VW::read_example(vw_obj, (char*)"1 | x");
    blank_line    = VW::read_example(vw_obj, (char*)"");
    VW::finish_example(vw_obj, blank_line);
    HookTask::task_data* d = srn.get_task_data<HookTask::task_data>();
    d->run_f = _searn_run_fn;
    d->run_object = this;
    d->var_map = NULL; // TODO
    //d->num_actions = num_actions;  // TODO
    d->extra_data  = NULL;
    d->extra_data2 = NULL;
  }
  ~SearchTask() { VW::finish_example(vw_obj, bogus_example); }

  virtual void _run(Searn::searn&srn, INPUT& input_example, OUTPUT& output) {}  // YOU MUST DEFINE THIS FUNCTION!

  void learn(INPUT& input_example, OUTPUT& output) {
    HookTask::task_data* d = srn.template get_task_data<HookTask::task_data> (); // ugly cvw_objing convention :(
    bogus_example->test_only = false;
    d->extra_data  = (void*)&input_example;
    d->extra_data2 = (void*)&output;
    vw_obj.learn(bogus_example);
    vw_obj.learn(blank_line);   // this will cause our searn_run_fn hook to get cvw_objed
  }

  void predict(INPUT& input_example, OUTPUT& output) {
    HookTask::task_data* d = srn.template get_task_data<HookTask::task_data> (); // ugly cvw_objing convention :(
    bogus_example->test_only = true;
    d->extra_data  = (void*)&input_example;
    d->extra_data2 = (void*)&output;
    vw_obj.learn(bogus_example);
    vw_obj.learn(blank_line);   // this will cause our searn_run_fn hook to get cvw_objed
  }
  

  protected:
  vw& vw_obj;
  Searn::searn& srn;
  
  private:
  example* bogus_example, *blank_line;

  static void _searn_run_fn(Searn::searn&srn) {
    HookTask::task_data* d = srn.get_task_data<HookTask::task_data>();
    if ((d->run_object == NULL) || (d->extra_data == NULL) || (d->extra_data2 == NULL)) {
      cerr << "error: cvw_objing _searn_run_fn without setting run object" << endl;
      throw exception();
    }
    ((SearchTask*)d->run_object)->_run(srn, *(INPUT*)d->extra_data, *(OUTPUT*)d->extra_data2);
  }
  
};


#endif
