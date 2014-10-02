/*
CoPyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_multiclasstask.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"
#include "ezexample.h"

namespace MulticlassTask { Search::search_task task = { "multiclasstask", run, initialize, finish, NULL, NULL };  }

namespace MulticlassTask {
  size_t max_label = 0;

  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    sch.set_options( 0 );
    sch.set_num_learners(ceil(num_actions/2));
    max_label = num_actions;
  }

  void finish(Search::search& sch) { }    // if we had task data, we'd want to free it here

  void run(Search::search& sch, vector<example*>& ec) {
    size_t gold_label = MULTICLASS::get_example_label(ec[0]);
    size_t tmp = ceil(log(max_label) /log(2))-1;
    size_t label = 0;
    size_t learner_id = 0;
    v_array<uint32_t> y_allowed;
    y_allowed.push_back(1);
    y_allowed.push_back(2);
    
    while(learner_id*2 <= max_label){
      action oracle = ((gold_label-1) >>tmp)%2+1;
      size_t prediction = sch.predict(*ec[0], 0, &oracle, 1, NULL, NULL, y_allowed.begin, 2, learner_id); // TODO: do we really need y_allowed?
      learner_id= learner_id * 2 + prediction;
      label = label*2 +(prediction -1);
      tmp--;
    }
    label+=1;
    sch.loss(!(label == gold_label));
    if (sch.output().good())
        sch.output() << label << ' ';
  }
}
