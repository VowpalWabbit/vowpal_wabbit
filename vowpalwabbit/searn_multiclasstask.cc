/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_multiclasstask.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"
#include "ezexample.h"

namespace MulticlassTask         {  Searn::searn_task task = { "multiclasstask",         initialize, finish, structured_predict };  }

namespace MulticlassTask {
  using namespace Searn;
  size_t max_label = 0;

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    srn.set_options( 0 );
    srn.set_num_learners(ceil(num_actions/2));
    max_label = num_actions;
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  void structured_predict(searn& srn, vector<example*> ec) {
    size_t gold_label = MULTICLASS::get_example_label(ec[0]);
    size_t tmp = ceil(log(max_label) /log(2))-1;
    size_t label = 0;
    size_t learner_id = 0;
    v_array<uint32_t> y_allowed;
    y_allowed.push_back(1);
    y_allowed.push_back(2);
    
    while(learner_id*2 <= max_label){
      srn.snapshot(learner_id, 1, &learner_id, sizeof(learner_id), true);
      srn.snapshot(tmp, 2, &tmp, sizeof(tmp), true);
      srn.snapshot(label, 3, &label, sizeof(label), true);
      size_t prediction = srn.predict(ec[0], ((gold_label-1) >>tmp)%2+1, &y_allowed, learner_id);
      learner_id= learner_id * 2 + prediction;
      label = label*2 +(prediction -1);
      tmp--;
    }
    label+=1;
    srn.loss(!(label == gold_label));
    if (srn.output().good())
        srn.output() << label << ' ';
  }
}


