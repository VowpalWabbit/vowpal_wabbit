/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_dep_parser.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"
#include "ezexample.h"

namespace DepParserTask         {  Searn::searn_task task = { "depparsertask",         initialize, finish, structured_predict };  }

  struct task_data {
  };
namespace DepParserTask {
  using namespace Searn;
  enum dep_action {SHIFT, RIGHT, LEFT, REDUCE};
  size_t max_label = 0;

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    srn.set_options( 0 );
    srn.set_num_learners(ceil(num_actions/2));
    max_label = num_actions;
    srn.set_num_learners(1);
    srn.set_task_data<task_data>(my_task_data);

  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  // function implemented an arc-standard System.
  size_t transition_standard(dep_action a_id, size_t i, v_array<size_t> & stack, v_array<size_t> & heads){
	if (a_id == SHIFT) {
		stack.push_back(i);
		return i+1;
	} else if (a_id == LEFT) {
		heads[stack.size()-1] = stack[stack.size()-2];
		stack.pop();
		return i;
	} else if (a_id == RIGHT) {
		heads[stack.size()-1] = i;
		stack.pop();
		return i;
	} else {
		cerr << "Unknown action (searn_dep_parser.cc).";
	}
  }
  void structured_predict(searn& srn, vector<example*> ec) {  
  }
}
