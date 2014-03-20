/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_sequencetask.h"
#include "oaa.h"
#include "example.h"

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
      clog << "task: calling snapshot i=" << i << endl;
      srn.snapshot(i, 1, &i, sizeof(i), true);
      clog << "task: return from snapshot i=" << i << endl;

      OAA::mc_label* y = (OAA::mc_label*)ec[i]->ld;
      clog << "task: asking for prediction @ " << i << endl;
      size_t prediction = srn.predict(ec[i], NULL, y->label);
      clog << "task: got prediction @ " << i << " = " << prediction << endl;

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (OAA::label_is_test(y) ? '?' : y->label) << ' ';
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
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won'td eclare_loss
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
      OAA::mc_label* y = (OAA::mc_label*)ec[i]->ld;
      size_t prediction = srn.predict(ec[i], &my_task_data->y_allowed, y->label);
      
      if (prediction == 1) sys_tag = 1;
      else sys_tag = ((prediction % 2) == 0) ? (uint32_t)(prediction+1) : (uint32_t)prediction;

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (OAA::label_is_test(y) ? '?' : y->label) << ' ';
    }
  }
}

namespace SequenceTask_DemoLDF {  // this is just to debug/show off how to do LDF
  using namespace Searn;

  struct task_data {
    example* ldf_examples;
    size_t   num_actions;
  };

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    CSOAA::wclass default_wclass = { 0., 0, 0., 0. };

    example* ldf_examples = alloc_examples(sizeof(CSOAA::label), num_actions);
    for (size_t a=0; a<num_actions; a++) {
      CSOAA::label* lab = (CSOAA::label*)ldf_examples[a].ld;
      lab->costs.push_back(default_wclass);
    }

    task_data* data = (task_data*)calloc(1, sizeof(task_data));
    data->ldf_examples = ldf_examples;
    data->num_actions  = num_actions;
    
    srn.task_data            = data;
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won't declare_loss
    srn.examples_dont_change = true;  // we don't do internal example munging -- this is okay because we explicitly copy all examples!
    srn.is_ldf               = true;  // we generate ldf examples
  }

  void finish(searn& srn) {
    task_data *data = (task_data*)srn.task_data;
    for (size_t a=0; a<data->num_actions; a++)
      dealloc_example(CSOAA::delete_label, data->ldf_examples[a]);
    free(data->ldf_examples);
    free(data);
  }

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    task_data *data = (task_data*)srn.task_data;
    
    for (size_t i=0; i<len; i++) { //save state for optimization
      srn.snapshot(i, 1, &i, sizeof(i), true);

      for (size_t a=0; a<data->num_actions; a++) {
        VW::copy_example_data(false, &data->ldf_examples[a], ec[i]);  // copy but leave label alone!

        // now, offset it appropriately for the action id
        update_example_indicies(true, &data->ldf_examples[a], quadratic_constant, cubic_constant * (uint32_t)a);
        
        // need to tell searn what the action id is, so that it can add history features correctly!
        CSOAA::label* lab = (CSOAA::label*)data->ldf_examples[a].ld;
        lab->costs[0].x = 0.;
        lab->costs[0].weight_index = (uint32_t)a+1;
        lab->costs[0].partial_prediction = 0.;
        lab->costs[0].wap_value = 0.;
      }

      OAA::mc_label* y = (OAA::mc_label*)ec[i]->ld;
      size_t pred_id = srn.predict(data->ldf_examples, data->num_actions, NULL, y->label - 1);
      size_t prediction = pred_id + 1;  // or ldf_examples[pred_it]->ld.costs[0].weight_index
      
      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (OAA::label_is_test(y) ? '?' : y->label) << ' ';
    }
  }

  void update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount) { // this is sort of bogus -- you'd never actually do this!
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; ++f)
        f->weight_index = (f->weight_index * mult_amount) + plus_amount;
    if (audit)
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
        if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
          for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; ++f)
            f->weight_index = (f->weight_index * mult_amount) + plus_amount;
  }
}
