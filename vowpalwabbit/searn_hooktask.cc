/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_hooktask.h"

// this is used for the C++ library and python library hook; hopefully
// it can be used for any foreign library too!
namespace HookTask {
  using namespace Searn;
  searn_task task = { "hook", initialize, finish, structured_predict };

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data *td = new task_data;
    td->run_f = NULL;
    td->run_object = NULL;
    td->delete_run_object = NULL;
    td->var_map = new po::variables_map(vm);
    td->num_actions = num_actions;
    srn.set_task_data<task_data>(td);
  }

  void finish(searn& srn) {
    task_data *td = srn.get_task_data<task_data>();
    cerr << "hook finish" << endl;
    if (td->run_object && td->delete_run_object) td->delete_run_object(td->run_object);
    delete td->var_map;
    delete td;
  }

  void structured_predict(searn& srn, vector<example*>& ec) {
    task_data *td = srn.get_task_data<task_data>();
    if (td->run_f)
      td->run_f(srn);
    else
      cerr << "warning: HookTask::structured_predict called before hook is set" << endl;
  }
}

