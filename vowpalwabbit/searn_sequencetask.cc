/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_sequencetask.h"
#include "oaa.h"

void label_to_array(void*label, v_array<uint32_t>&out) {
  OAA::mc_label* l = (OAA::mc_label*)label;
  if (l->label == (uint32_t)-1)
    out.erase();
  else {
    if (out.size() == 1) out[0] = l->label;
    else {
      out.erase();
      out.push_back( l->label );
    }
  }
}

namespace SequenceTask {
  using namespace Searn;

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    srn.task_data            = new v_array<uint32_t>();
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) {
    v_array<uint32_t> * y_star = (v_array<uint32_t>*) srn.task_data;
    y_star->erase();
    y_star->delete_v();
    delete y_star;
  }

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    v_array<uint32_t> * y_star = (v_array<uint32_t>*) srn.task_data;
    float total_loss  = 0;

    for (size_t i=0; i<len; i++) { //save state for optimization
      srn.snapshot(i, 1, &i, sizeof(i), true);
      srn.snapshot(i, 2, &total_loss, sizeof(total_loss), false);

      //predict with label advice
      label_to_array(ec[i]->ld, *y_star);
      size_t prediction = srn.predict(ec[i], NULL, y_star);

      //track loss
      if (y_star->size() > 0)
        total_loss += (float)(prediction != y_star->last());

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << ((y_star->size() == 0) ? '?' : y_star->last()) << ' ';
    }//declare loss
    srn.declare_loss(len, total_loss);
  }
}

namespace SequenceSpanTask {
  using namespace Searn;

  struct task_data {
    v_array<uint32_t> y_allowed;
    v_array<uint32_t> y_star;
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
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) {
    task_data * my_task_data = (task_data*)srn.task_data;
    my_task_data->y_allowed.erase();    my_task_data->y_allowed.delete_v();
    my_task_data->y_star.erase();       my_task_data->y_star.delete_v();
    delete my_task_data;
  }

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    task_data * my_task_data = (task_data*)srn.task_data;
    float total_loss  = 0;
    uint32_t sys_tag = 1;
    
    for (size_t i=0; i<len; i++) {
      srn.snapshot(i, 1, &i, sizeof(i), true);
      srn.snapshot(i, 2, &sys_tag, sizeof(sys_tag), true);
      srn.snapshot(i, 3, &total_loss, sizeof(total_loss), false);

      label_to_array(ec[i]->ld, my_task_data->y_star);
      my_task_data->y_allowed[my_task_data->y_allowed.size()-1] = sys_tag;
      size_t prediction = srn.predict(ec[i], &my_task_data->y_allowed, &my_task_data->y_star);

      if (prediction == 1) sys_tag = 1;
      else sys_tag = ((prediction % 2) == 0) ? (uint32_t)(prediction+1) : (uint32_t)prediction;
      
      if (my_task_data->y_star.size() > 0)
        total_loss += (float)(prediction != my_task_data->y_star[0]);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << ((my_task_data->y_star.size() == 0) ? '?' : my_task_data->y_star[0]) << ' ';
    }
    srn.declare_loss(len, total_loss);
  }
}







