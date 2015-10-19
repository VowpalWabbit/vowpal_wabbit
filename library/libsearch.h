#/*
COpyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef LIBSEARCH_HOOKTASK_H
#define LIBSEARCH_HOOKTASK_H

#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/parse_example.h"
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/search.h"
#include "../vowpalwabbit/search_hooktask.h"

using namespace std;

template<class INPUT, class OUTPUT> class SearchTask {
  public:
  SearchTask(vw& vw_obj) : vw_obj(vw_obj), sch(*(Search::search*)vw_obj.searchstr) {
    bogus_example = VW::alloc_examples(vw_obj.p->lp.label_size, 1);
    VW::read_line(vw_obj, bogus_example, (char*)"1 | x");
    VW::parse_atomic_example(vw_obj, bogus_example, false);
    VW::setup_example(vw_obj, bogus_example);

    blank_line = VW::alloc_examples(vw_obj.p->lp.label_size, 1);
    VW::read_line(vw_obj, blank_line, (char*)"");
    VW::parse_atomic_example(vw_obj, blank_line, false);
    VW::setup_example(vw_obj, blank_line);

    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    d->run_f = _search_run_fn;
    d->run_setup_f = _search_setup_fn;
    d->run_takedown_f = _search_takedown_fn;
    d->run_object = this;
    d->extra_data  = NULL;
    d->extra_data2 = NULL;
  }
  ~SearchTask() {
    VW::dealloc_example(vw_obj.p->lp.delete_label, *bogus_example); free(bogus_example);
    VW::dealloc_example(vw_obj.p->lp.delete_label, *blank_line);    free(blank_line);
  }

  virtual void _run(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // YOU MUST DEFINE THIS FUNCTION!
  void       _setup(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // OPTIONAL
  void    _takedown(Search::search&sch, INPUT& input_example, OUTPUT& output) {}  // OPTIONAL

  void   learn(INPUT& input_example, OUTPUT& output) { bogus_example->test_only = false; call_vw(input_example, output); }
  void predict(INPUT& input_example, OUTPUT& output) { bogus_example->test_only = true;  call_vw(input_example, output); }

  protected:
  vw& vw_obj;
  Search::search& sch;

  private:
  example* bogus_example, *blank_line;

  void call_vw(INPUT& input_example, OUTPUT& output) {
    HookTask::task_data* d = sch.template get_task_data<HookTask::task_data> (); // ugly calling convention :(
    d->extra_data  = (void*)&input_example;
    d->extra_data2 = (void*)&output;
    vw_obj.learn(bogus_example);
    vw_obj.learn(blank_line);   // this will cause our search_run_fn hook to get called
  }

  static void _search_run_fn(Search::search&sch) {
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if ((d->run_object == NULL) || (d->extra_data == NULL) || (d->extra_data2 == NULL)) {
      cerr << "error: calling _search_run_fn without setting run object" << endl;
      throw exception();
    }
    ((SearchTask*)d->run_object)->_run(sch, *(INPUT*)d->extra_data, *(OUTPUT*)d->extra_data2);
  }

  static void _search_setup_fn(Search::search&sch) {
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if ((d->run_object == NULL) || (d->extra_data == NULL) || (d->extra_data2 == NULL)) {
      cerr << "error: calling _search_setup_fn without setting run object" << endl;
      throw exception();
    }
    ((SearchTask*)d->run_object)->_setup(sch, *(INPUT*)d->extra_data, *(OUTPUT*)d->extra_data2);
  }

  static void _search_takedown_fn(Search::search&sch) {
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    if ((d->run_object == NULL) || (d->extra_data == NULL) || (d->extra_data2 == NULL)) {
      cerr << "error: calling _search_takedown_fn without setting run object" << endl;
      throw exception();
    }
    ((SearchTask*)d->run_object)->_takedown(sch, *(INPUT*)d->extra_data, *(OUTPUT*)d->extra_data2);
  }

};


class BuiltInTask : public SearchTask< vector<example*>, vector<uint32_t> > {
  public:
  BuiltInTask(vw& vw_obj, Search::search_task* task)
      : SearchTask< vector<example*>, vector<uint32_t> >(vw_obj) {
    HookTask::task_data* d = sch.get_task_data<HookTask::task_data>();
    size_t num_actions = d->num_actions;
    my_task = task;
    if (my_task->initialize)
      my_task->initialize(sch, num_actions, *d->var_map);
  }

  ~BuiltInTask() { if (my_task->finish) my_task->finish(sch); }

  void _run(Search::search& sch, vector<example*> & input_example, vector<uint32_t> & output) {
    my_task->run(sch, input_example);
    sch.get_test_action_sequence(output);
  }

  protected:
  Search::search_task* my_task;
};


#endif
