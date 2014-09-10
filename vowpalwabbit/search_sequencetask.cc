/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_sequencetask.h"
#include "multiclass.h"      // needed for non-LDF
#include "cost_sensitive.h"  // needed for LDF

namespace SequenceTask2 { Search::search_task task = { "sequence", run, initialize, NULL, NULL, NULL }; }
namespace SequenceTask_DemoLDF2 { Search::search_task task = { "sequenceldf", run, initialize, finish, NULL, NULL }; }

namespace SequenceTask2 {
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    sch.set_options( Search::AUTO_CONDITION_FEATURES  |    // automatically add history features to our examples, please
                     Search::AUTO_HAMMING_LOSS        |    // please just use hamming loss on individual predictions -- we won't declare loss
                     0); //Search::EXAMPLES_DONT_CHANGE     );   // we don't do any internal example munging
  }

  void run(Search::search& sch, vector<example*>& ec) {
    for (int i=0; i<ec.size(); i++) {
      ptag last_tags[2] = { max(0,i-1), i };
      action oracle     = MULTICLASS::get_example_label(ec[i]);
      size_t prediction = sch.predict(*ec[i],    // predict using features from ec[i]
                                      i+1,       // our "tag" is i+1 (because tags are 1-based)
                                      &oracle,   // this is the (only) oracle action
                                      1,         // there is only one oracle action
                                      last_tags, // condition on the previous _prediction_
                                      "qp");     // call the conditioning 'p' for "previous" and 'q' for prevprev

      if (sch.output().good())
        sch.output() << prediction << ' ';
    }
  }
}


namespace SequenceTask_DemoLDF2 {  // this is just to debug/show off how to do LDF
  namespace CS=COST_SENSITIVE;
  struct task_data {
    example* ldf_examples;
    size_t   num_actions;
  };
  
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    CS::wclass default_wclass = { 0., 0, 0., 0. };

    example* ldf_examples = alloc_examples(sizeof(CS::label), num_actions);
    for (size_t a=0; a<num_actions; a++) {
      CS::label* lab = (CS::label*)ldf_examples[a].ld;
      CS::cs_label.default_label(lab);
      lab->costs.push_back(default_wclass);
    }

    task_data* data = (task_data*)calloc(1, sizeof(task_data));
    data->ldf_examples = ldf_examples;
    data->num_actions  = num_actions;

    sch.set_task_data<task_data>(data);
    sch.set_options( Search::AUTO_CONDITION_FEATURES |    // automatically add history features to our examples, please
                     Search::AUTO_HAMMING_LOSS       |    // please just use hamming loss on individual predictions -- we won't declare loss
                     Search::IS_LDF                  );   // we generate ldf examples
  }

  void finish(Search::search& sch) {
    task_data *data = sch.get_task_data<task_data>();
    for (size_t a=0; a<data->num_actions; a++)
      dealloc_example(CS::cs_label.delete_label, data->ldf_examples[a]);
    free(data->ldf_examples);
    free(data);
  }


  // this is totally bogus for the example -- you'd never actually do this!
  void my_update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount) {
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; ++f)
        f->weight_index = (f->weight_index * mult_amount) + plus_amount;
    if (audit)
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
        if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
          for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; ++f)
            f->weight_index = (f->weight_index * mult_amount) + plus_amount;
  }
  
  void run(Search::search& sch, vector<example*>& ec) {
    task_data *data = sch.get_task_data<task_data>();
    
    for (ptag i=0; i<ec.size(); i++) { //save state for optimization
      for (size_t a=0; a<data->num_actions; a++) {
        VW::copy_example_data(false, &data->ldf_examples[a], ec[i]);  // copy but leave label alone!

        // now, offset it appropriately for the action id
        my_update_example_indicies(true, &data->ldf_examples[a], 28904713, 4832917 * (uint32_t)a);
        
        // need to tell search what the action id is, so that it can add history features correctly!
        CS::label* lab = (CS::label*)data->ldf_examples[a].ld;
        lab->costs[0].x = 0.;
        lab->costs[0].class_index = (uint32_t)a+1;
        lab->costs[0].partial_prediction = 0.;
        lab->costs[0].wap_value = 0.;
      }

      action oracle  = MULTICLASS::get_example_label(ec[i]) - 1;
      action pred_id = sch.predictLDF(data->ldf_examples,
                                      data->num_actions,
                                      i+1,
                                      &oracle,
                                      1,
                                      &i,
                                      "p");
      action prediction = pred_id + 1;  // or ldf_examples[pred_id]->ld.costs[0].weight_index
      
      if (sch.output().good())
        sch.output() << prediction << ' ';
    }
  }
}
