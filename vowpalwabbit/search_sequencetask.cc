/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_sequencetask.h"
#include "multiclass.h"      // needed for non-LDF
#include "cost_sensitive.h"  // needed for LDF

namespace SequenceTask2 { Search::search_task task = { "sequence", run, initialize, NULL, NULL, NULL }; }
namespace SequenceSpanTask2 { Search::search_task task = { "sequencespan",  run, initialize, finish, NULL, NULL };  }
namespace SequenceTask_DemoLDF2 { Search::search_task task = { "sequenceldf", run, initialize, finish, NULL, NULL }; }

namespace SequenceTask2 {
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    sch.set_options( Search::AUTO_CONDITION_FEATURES  |    // automatically add history features to our examples, please
                     Search::AUTO_HAMMING_LOSS        |    // please just use hamming loss on individual predictions -- we won't declare loss
                     Search::EXAMPLES_DONT_CHANGE     |    // we don't do any internal example munging
                     0);
  }

  void run(Search::search& sch, vector<example*>& ec) {
    for (int i=0; i<ec.size(); i++) {
      ptag last_tag = i;
      //ptag last_tags[2] = { max(0,i-1), i };
      action oracle     = MULTICLASS::get_example_label(ec[i]);
      size_t prediction = sch.predict(*ec[i],    // predict using features from ec[i]
                                      i+1,       // our "tag" is i+1 (because tags are 1-based)
                                      &oracle,   // this is the (only) oracle action
                                      1,         // there is only one oracle action
                                      &last_tag, "p");
                                      //last_tags, // condition on the previous _prediction_
                                      //"qp");     // call the conditioning 'p' for "previous" and 'q' for prevprev

      if (sch.output().good())
        sch.output() << prediction << ' ';
    }
  }
}


namespace SequenceSpanTask2 {
  enum EncodingType { BIO, BILOU };
// the format for the BIO encoding is:
//     label     description
//     1         "O" (out)
//     n even    begin X, where X is defined by n/2
//     n odd     in X, where X is (n-1)/2
//   thus, valid transitions are:
//     *       -> 1       (anything to OUT)
//     *       -> n even  (anything in BEGIN X)
//     n even  -> n+1     (BEGIN X to IN X)
//     n odd>1 -> n       (IN X to IN X)
// the format for the BILOU (begin, inside, last, out, unit-length) encoding is:
//     label     description
//     1         out
//     n>1: let m=n-2:
//       m % 4 == 0    unit-(m div 4)
//       m % 4 == 1    begin-(m div 4)
//       m % 4 == 2    in-(m div 4)
//       m % 4 == 3    last-(m div 4)
//   thus, valid transitions are:
//     1     -> 1; 2, 6, 8, ...; 3, 7, 9, ...         out to { out, unit-Y, begin-Y }       1
//     m%4=0 -> 1; 2, 6, 8, ..., 3, 7, 9, ...         unit-X to { out, unit-Y, begin-Y }    2, 6, 10, 14, ...
//     m%4=1 -> m+1, m+2                              begin-X to { in-X, last-X }           3, 7, 11, 15, ...
//     m%4=2 -> m, m+1                                in-X to { in-X, last-X }              4, 8, 12, 16, ...
//     m%4=3 -> 1; 2, 6, 8, ...; 3, 7, 9, ...         last-X to { out, unit-Y, begin-Y }    5, 9, 13, 17, ...

  void convert_bio_to_bilou(vector<example*> ec) {
    for (size_t n=0; n<ec.size(); n++) {
      MULTICLASS::multiclass* ylab = (MULTICLASS::multiclass*)ec[n]->ld;
      action y = ylab->label;
      action nexty = (n == ec.size()-1) ? 0 : ((MULTICLASS::multiclass*)ec[n+1]->ld)->label;
      if (y == 1) { // do nothing
      } else if (y % 2 == 0) { // this is a begin-X
        if (nexty != y + 1) // should be unit
          ylab->label = (y/2 - 1) * 4 + 2;  // from 2 to 2, 4 to 6, 6 to 10, etc.
        else // should be begin-X
          ylab->label = (y/2 - 1) * 4 + 3;  // from 2 to 3, 4 to 7, 6 to 11, etc.
      } else if (y % 2 == 1) { // this is an in-X
        if (nexty != y) // should be last
          ylab->label = (y-1) * 2 + 1;  // from 3 to 5, 5 to 9, 7 to 13, etc.
        else // should be in-X
          ylab->label = (y-1) * 2;      // from 3 to 4, 5 to 8, 7 to 12, etc.
      }
      assert(ylab->label <= 13);
    }
  }

  inline action bilou_to_bio(action y) {
    return y / 2 + 1;  // out -> out, {unit,begin} -> begin; {in,last} -> in
  }

  struct task_data {
    EncodingType encoding;
    v_array<action> allowed_actions;
    v_array<action> only_two_allowed;  // used for BILOU encoding
  };

  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();

    // TODO: options!
    // po::options_description sspan_opts("search sequencespan options");
    // sspan_opts.add_options()("search_span_bilou", "switch to (internal) BILOU encoding instead of BIO encoding");
    // vm = add_options(*sch.all, sspan_opts);
    // if (vm.count("search_span_bilou")) {
    //   cerr << "switching to BILOU encoding for sequence span labeling" << endl;
    //   my_task_data->encoding = BILOU;
    //   num_actions = num_actions * 2 - 1;
    // } else
      my_task_data->encoding = BIO;
    
    
    my_task_data->allowed_actions.erase();

    if (my_task_data->encoding == BIO) {
      my_task_data->allowed_actions.push_back(1);
      for (action l=2; l<num_actions; l+=2)
        my_task_data->allowed_actions.push_back(l);
      my_task_data->allowed_actions.push_back(1);  // push back an extra 1 that we can overwrite later if we want
    } else if (my_task_data->encoding == BILOU) {
      my_task_data->allowed_actions.push_back(1);
      for (action l=2; l<num_actions; l+=4) {
        my_task_data->allowed_actions.push_back(l);
        my_task_data->allowed_actions.push_back(l+1);
      }
      my_task_data->only_two_allowed.push_back(0);
      my_task_data->only_two_allowed.push_back(0);
    }

    sch.set_task_data<task_data>(my_task_data);
    sch.set_options( Search::AUTO_CONDITION_FEATURES  |    // automatically add history features to our examples, please
                     Search::AUTO_HAMMING_LOSS        |    // please just use hamming loss on individual predictions -- we won't declare loss
                     Search::EXAMPLES_DONT_CHANGE     |    // we don't do any internal example munging
                     0);
  }

  void finish(Search::search& sch) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    my_task_data->allowed_actions.delete_v();
    my_task_data->only_two_allowed.delete_v();
    delete my_task_data;
  }

  void run(Search::search& sch, vector<example*>& ec) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    action last_prediction = 1;
    v_array<action> * y_allowed = &(my_task_data->allowed_actions);

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      convert_bio_to_bilou(ec);
    
    for (size_t i=0; i<ec.size(); i++) {
      if (my_task_data->encoding == BIO) {
        if      (last_prediction == 1)      (*y_allowed)[y_allowed->size()-1] = 1;
        else if (last_prediction % 2 == 0)  (*y_allowed)[y_allowed->size()-1] = last_prediction+1;
        else                                (*y_allowed)[y_allowed->size()-1] = last_prediction;
      } else if (my_task_data->encoding == BILOU) {
        if ((last_prediction == 1) || ((last_prediction-2) % 4 == 0) || ((last_prediction-2) % 4 == 3))
          y_allowed = &(my_task_data->allowed_actions);
        else {
          y_allowed = &(my_task_data->only_two_allowed);
          my_task_data->only_two_allowed[0] = last_prediction+1;
          my_task_data->only_two_allowed[1] = ((last_prediction-2) % 4 == 1) ? (last_prediction+2) : last_prediction;
        }
      }

      ptag last_tag = i;
      action oracle = MULTICLASS::get_example_label(ec[i]);
      //last_prediction = sch.predict(ec[i], MULTICLASS::get_example_label(ec[i]), y_allowed);
      last_prediction = sch.predict(*ec[i],
                                    i+1,
                                    &oracle,
                                    1,
                                    &last_tag,
                                    "p");

      action printed_prediction = (my_task_data->encoding == BIO) ? last_prediction : bilou_to_bio(last_prediction);
      
      if (sch.output().good())
        sch.output() << printed_prediction << ' ';
    }

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      for (size_t n=0; n<ec.size(); n++) {
        MULTICLASS::multiclass* ylab = (MULTICLASS::multiclass*)ec[n]->ld;
        ylab->label = bilou_to_bio(ylab->label);
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
