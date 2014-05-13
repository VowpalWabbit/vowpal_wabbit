/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "searn_sequencetask.h"
#include "multiclass.h"
#include "memory.h"
#include "example.h"
#include "gd.h"

namespace SequenceTask         {  Searn::searn_task task = { "sequence",         initialize, finish, structured_predict };  }
namespace ArgmaxTask        {  Searn::searn_task task = { "argmax",        initialize, finish, structured_predict };  }
namespace SequenceTask_DemoLDF {  Searn::searn_task task = { "sequence_demoldf", initialize, finish, structured_predict };  }
namespace SequenceSpanTask     {  Searn::searn_task task = { "sequencespan",     initialize, finish, structured_predict };  }


namespace SequenceTask {
  using namespace Searn;

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    srn.set_options( AUTO_HISTORY         |    // automatically add history features to our examples, please
                     AUTO_HAMMING_LOSS    |    // please just use hamming loss on individual predictions -- we won't declare loss
                     EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  void structured_predict(searn& srn, vector<example*> ec) {
    for (size_t i=0; i<ec.size(); i++) { //save state for optimization
      srn.snapshot(i, 1, &i, sizeof(i), true);

      size_t prediction = srn.predict(ec[i], MULTICLASS::get_example_label(ec[i]));

      if (srn.output().good())
        srn.output() << prediction << ' ';
    }
  }
}


namespace ArgmaxTask {
  using namespace Searn;

  struct task_data {
    float false_negative_cost;
    bool predict_max;
  };

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) 
  {
    
    task_data* my_task_data = new task_data();
    
    po::options_description argmax_opts("argmax options");
    argmax_opts.add_options()
      ("cost", po::value<float>(&(my_task_data->false_negative_cost))->default_value(10.0), "False Negative Cost")
      ("max", "Disable structure: just predict the max");

    vm = add_options(*srn.all, argmax_opts);

    if (vm.count("max"))
      my_task_data->predict_max = true;
    else
      my_task_data->predict_max = false;      
	    
    srn.set_task_data(my_task_data);

    if (my_task_data->predict_max)
      srn.set_options( EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
    else
      srn.set_options( AUTO_HISTORY         |    // automatically add history features to our examples, please
		       EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  void structured_predict(searn& srn, vector<example*> ec) {
    task_data * my_task_data = srn.get_task_data<task_data>();
    uint32_t max_prediction = 1;
    uint32_t max_label = 1;

    for(size_t i = 0; i < ec.size(); i++)
      max_label = max(MULTICLASS::get_example_label(ec[i]), max_label);
        
    for (size_t i=0; i<ec.size(); i++) {
      // labels should be 1 or 2, and our output is MAX of all predicted values
      srn.snapshot(i, 1, &i, sizeof(i), true); //save state for optimization
      srn.snapshot(i, 2, &max_prediction, sizeof(max_prediction), false); 

      uint32_t prediction;
      if (my_task_data->predict_max)
	prediction = srn.predict(ec[i], max_label);
      else
	prediction = srn.predict(ec[i], MULTICLASS::get_example_label(ec[i]));

      max_prediction = max(prediction, max_prediction);
    }
    float loss = 0.;
    if (max_label > max_prediction)
      loss = my_task_data->false_negative_cost;
    else if (max_prediction > max_label)
      loss = 1.;		
    srn.loss(loss);

    if (srn.output().good())
      srn.output() << max_prediction;
  }
}

namespace SequenceSpanTask {
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

  using namespace Searn;

  void convert_bio_to_bilou(vector<example*> ec) {
    for (size_t n=0; n<ec.size(); n++) {
      MULTICLASS::multiclass* ylab = (MULTICLASS::multiclass*)ec[n]->ld;
      uint32_t y = ylab->label;
      uint32_t nexty = (n == ec.size()-1) ? 0 : ((MULTICLASS::multiclass*)ec[n+1]->ld)->label;
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

  inline uint32_t bilou_to_bio(uint32_t y) {
    return y / 2 + 1;  // out -> out, {unit,begin} -> begin; {in,last} -> in
  }

  struct task_data {
    EncodingType encoding;
    v_array<uint32_t> y_allowed;
    v_array<uint32_t> only_two_allowed;  // used for BILOU encoding
  };

  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();

    po::options_description sspan_opts("search sequencespan options");
    sspan_opts.add_options()("search_span_bilou", "switch to (internal) BILOU encoding instead of BIO encoding");

    vm = add_options(*srn.all, sspan_opts);

    if (vm.count("search_span_bilou")) {
      cerr << "switching to BILOU encoding for sequence span labeling" << endl;
      my_task_data->encoding = BILOU;
      num_actions = num_actions * 2 - 1;
    } else
      my_task_data->encoding = BIO;
    
    
    my_task_data->y_allowed.erase();

    if (my_task_data->encoding == BIO) {
      my_task_data->y_allowed.push_back(1);
      for (uint32_t l=2; l<num_actions; l+=2)
        my_task_data->y_allowed.push_back(l);
      my_task_data->y_allowed.push_back(1);  // push back an extra 1 that we can overwrite later if we want
    } else if (my_task_data->encoding == BILOU) {
      my_task_data->y_allowed.push_back(1);
      for (uint32_t l=2; l<num_actions; l+=4) {
        my_task_data->y_allowed.push_back(l);
        my_task_data->y_allowed.push_back(l+1);
      }
      my_task_data->only_two_allowed.push_back(0);
      my_task_data->only_two_allowed.push_back(0);
    }

    srn.set_task_data<task_data>(my_task_data);
    srn.set_options( AUTO_HISTORY         |    // automatically add history features to our examples, please
                     AUTO_HAMMING_LOSS    |    // please just use hamming loss on individual predictions -- we won't declare loss
                     EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
  }

  void finish(searn& srn) {
    task_data * my_task_data = srn.get_task_data<task_data>();
    my_task_data->y_allowed.delete_v();
    my_task_data->only_two_allowed.delete_v();
    delete my_task_data;
  }

  void structured_predict(searn& srn, vector<example*> ec) {
    task_data * my_task_data = srn.get_task_data<task_data>();
    uint32_t last_prediction = 1;
    v_array<uint32_t> * y_allowed = &(my_task_data->y_allowed);

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      convert_bio_to_bilou(ec);
    
    for (size_t i=0; i<ec.size(); i++) {
      srn.snapshot(i, 1, &i, sizeof(i), true);
      srn.snapshot(i, 2, &last_prediction, sizeof(last_prediction), true);

      if (my_task_data->encoding == BIO) {
        if      (last_prediction == 1)      (*y_allowed)[y_allowed->size()-1] = 1;
        else if (last_prediction % 2 == 0)  (*y_allowed)[y_allowed->size()-1] = last_prediction+1;
        else                                (*y_allowed)[y_allowed->size()-1] = last_prediction;
      } else if (my_task_data->encoding == BILOU) {
        if ((last_prediction == 1) || ((last_prediction-2) % 4 == 0) || ((last_prediction-2) % 4 == 3))
          y_allowed = &(my_task_data->y_allowed);
        else {
          y_allowed = &(my_task_data->only_two_allowed);
          my_task_data->only_two_allowed[0] = last_prediction+1;
          my_task_data->only_two_allowed[1] = ((last_prediction-2) % 4 == 1) ? (last_prediction+2) : last_prediction;
        }
      }
      last_prediction = srn.predict(ec[i], MULTICLASS::get_example_label(ec[i]), y_allowed);

      uint32_t printed_prediction = (my_task_data->encoding == BIO) ? last_prediction : bilou_to_bio(last_prediction);
      //uint32_t printed_truth      = (my_task_data->encoding == BIO) ? y->label        : bilou_to_bio(y->label);
      
      if (srn.output().good())
        srn.output() << printed_prediction << ' ';
    }

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      for (size_t n=0; n<ec.size(); n++) {
        MULTICLASS::multiclass* ylab = (MULTICLASS::multiclass*)ec[n]->ld;
        ylab->label = bilou_to_bio(ylab->label);
      }
  }
}

namespace SequenceTask_DemoLDF {  // this is just to debug/show off how to do LDF
  using namespace Searn;

  struct task_data {
    example* ldf_examples;
    size_t   num_actions;
  };
  
  void initialize(searn& srn, size_t& num_actions, po::variables_map& vm) {
    COST_SENSITIVE::wclass default_wclass = { 0., 0, 0., 0. };

    example* ldf_examples = alloc_examples(sizeof(COST_SENSITIVE::label), num_actions);
    for (size_t a=0; a<num_actions; a++) {
      COST_SENSITIVE::label* lab = (COST_SENSITIVE::label*)ldf_examples[a].ld;
      lab->costs.push_back(default_wclass);
    }

    task_data* data = (task_data*)calloc_or_die(1, sizeof(task_data));
    data->ldf_examples = ldf_examples;
    data->num_actions  = num_actions;

    srn.set_task_data<task_data>(data);
    srn.set_options( AUTO_HISTORY         |    // automatically add history features to our examples, please
                     AUTO_HAMMING_LOSS    |    // please just use hamming loss on individual predictions -- we won't declare loss
                     IS_LDF               );   // we generate ldf examples
  }

  void finish(searn& srn) {
    task_data *data = srn.get_task_data<task_data>();
    for (size_t a=0; a<data->num_actions; a++)
      dealloc_example(COST_SENSITIVE::cs_label.delete_label, data->ldf_examples[a]);
    free(data->ldf_examples);
    free(data);
  }

  void structured_predict(searn& srn, vector<example*> ec) {
    task_data *data = srn.get_task_data<task_data>();
    
    for (size_t i=0; i<ec.size(); i++) { //save state for optimization
      srn.snapshot(i, 1, &i, sizeof(i), true);

      for (size_t a=0; a<data->num_actions; a++) {
        VW::copy_example_data(false, &data->ldf_examples[a], ec[i]);  // copy but leave label alone!

        // now, offset it appropriately for the action id
        update_example_indicies(true, &data->ldf_examples[a], quadratic_constant, cubic_constant * (uint32_t)a);
        
        // need to tell searn what the action id is, so that it can add history features correctly!
        COST_SENSITIVE::label* lab = (COST_SENSITIVE::label*)data->ldf_examples[a].ld;
        lab->costs[0].x = 0.;
        lab->costs[0].class_index = (uint32_t)a+1;
        lab->costs[0].partial_prediction = 0.;
        lab->costs[0].wap_value = 0.;
      }
      
      size_t pred_id = srn.predict(data->ldf_examples, data->num_actions, MULTICLASS::get_example_label(ec[i]) - 1);
      size_t prediction = pred_id + 1;  // or ldf_examples[pred_id]->ld.costs[0].weight_index
      
      if (srn.output().good())
        srn.output() << prediction << ' ';
    }
  }

  // this is totally bogus for the example -- you'd never actually do this!
  void update_example_indicies(bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount) {
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
