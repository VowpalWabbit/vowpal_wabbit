/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_sequencetask.h"
#include "oaa.h"

namespace SequenceTask {
  using namespace Searn;

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    srn.task_data            = NULL;  // we don't need any of our own data
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won't declare_loss
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    for (size_t i=0; i<len; i++) { //save state for optimization
      srn.snapshot(i, 1, &i, sizeof(i), true);

      OAA::mc_label* label = (OAA::mc_label*)ec[i]->ld;
      size_t prediction = srn.predict(ec[i], NULL, label);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (OAA::label_is_test(label) ? '?' : label->label) << ' ';
    }
  }
}

namespace SequenceSpanTask {
  using namespace Searn;

  struct task_data {
    v_array<uint32_t> y_allowed;
  };

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    task_data * my_task_data = new task_data();
    my_task_data->y_allowed.erase();
    my_task_data->y_allowed.push_back(1);
    for (size_t l=2; l<num_actions; l+=2)
      my_task_data->y_allowed.push_back((uint32_t)l);
    my_task_data->y_allowed.push_back(1);  // push back an extra 1 that we can overwrite later if we want
    
    srn.task_data            = my_task_data;
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won't declare_loss
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) {
    task_data * my_task_data = (task_data*)srn.task_data;
    my_task_data->y_allowed.erase();    my_task_data->y_allowed.delete_v();
    delete my_task_data;
  }

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    task_data * my_task_data = (task_data*)srn.task_data;
    uint32_t sys_tag = 1;
    
    for (size_t i=0; i<len; i++) {
      srn.snapshot(i, 1, &i, sizeof(i), true);
      srn.snapshot(i, 2, &sys_tag, sizeof(sys_tag), true);

      my_task_data->y_allowed[my_task_data->y_allowed.size()-1] = sys_tag;
      OAA::mc_label* label = (OAA::mc_label*)ec[i]->ld;
      size_t prediction = srn.predict(ec[i], &my_task_data->y_allowed, label);
      
      if (prediction == 1) sys_tag = 1;
      else sys_tag = ((prediction % 2) == 0) ? (uint32_t)(prediction+1) : (uint32_t)prediction;

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (OAA::label_is_test(label) ? '?' : label->label) << ' ';
    }
  }
}







