/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_sequencetask.h"
#include "multiclass.h"      // needed for non-LDF
#include "cost_sensitive.h"  // needed for LDF
#include "vw.h"

namespace SequenceTask         { Search::search_task task = { "sequence",          run, initialize, NULL,   NULL,  NULL     }; }
namespace SequenceSpanTask     { Search::search_task task = { "sequencespan",      run, initialize, finish, setup, takedown }; }
namespace ArgmaxTask           { Search::search_task task = { "argmax",            run, initialize, NULL,   NULL,  NULL     }; }
namespace SequenceTask_DemoLDF { Search::search_task task = { "sequence_demoldf",  run, initialize, finish, NULL,  NULL     }; }

namespace SequenceTask {
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    sch.set_options( Search::AUTO_CONDITION_FEATURES  |    // automatically add history features to our examples, please
                     Search::AUTO_HAMMING_LOSS        |    // please just use hamming loss on individual predictions -- we won't declare loss
                     Search::EXAMPLES_DONT_CHANGE     |    // we don't do any internal example munging
                     0);
  }

  void run(Search::search& sch, vector<example*>& ec) {
    for (size_t i=0; i<ec.size(); i++) {
      action oracle     = ec[i]->l.multi.label;
      size_t prediction = Search::predictor(sch, (ptag)i+1).set_input(*ec[i]).set_oracle(oracle).set_condition_range((ptag)i, sch.get_history_length(), 'p').predict();

      if (sch.output().good())
        sch.output() << prediction << ' ';
    }
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
//     1     -> 1; 2, 6, 10, ...; 3, 7, 11, ...         out to { out, unit-Y, begin-Y }       1
//     m%4=0 -> 1; 2, 6, 10, ..., 3, 7, 11, ...         unit-X to { out, unit-Y, begin-Y }    2, 6, 10, 14, ...
//     m%4=1 -> m+1, m+2                                begin-X to { in-X, last-X }           3, 7, 11, 15, ...
//     m%4=2 -> m, m+1                                  in-X to { in-X, last-X }              4, 8, 12, 16, ...
//     m%4=3 -> 1; 2, 6, 10, ...; 3, 7, 11, ...         last-X to { out, unit-Y, begin-Y }    5, 9, 13, 17, ...

  inline action bilou_to_bio(action y) {
    return y / 2 + 1;  // out -> out, {unit,begin} -> begin; {in,last} -> in
  }

  void convert_bio_to_bilou(vector<example*> ec) {
    for (size_t n=0; n<ec.size(); n++) {
      MULTICLASS::multiclass& ylab = ec[n]->l.multi;
      action y = ylab.label;
      action nexty = (n == ec.size()-1) ? 0 : ec[n+1]->l.multi.label;
      if (y == 1) { // do nothing
      } else if (y % 2 == 0) { // this is a begin-X
        if (nexty != y + 1) // should be unit
          ylab.label = (y/2 - 1) * 4 + 2;  // from 2 to 2, 4 to 6, 6 to 10, etc.
        else // should be begin-X
          ylab.label = (y/2 - 1) * 4 + 3;  // from 2 to 3, 4 to 7, 6 to 11, etc.
      } else if (y % 2 == 1) { // this is an in-X
        if (nexty != y) // should be last
          ylab.label = (y-1) * 2 + 1;  // from 3 to 5, 5 to 9, 7 to 13, etc.
        else // should be in-X
          ylab.label = (y-1) * 2;      // from 3 to 4, 5 to 8, 7 to 12, etc.
      }
      assert( y == bilou_to_bio(ylab.label) );
    }
  }

  struct task_data {
    EncodingType encoding;
    v_array<action> allowed_actions;
    v_array<action> only_two_allowed;  // used for BILOU encoding
  };

  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data * my_task_data = new task_data();
    po::options_description sspan_opts("search sequencespan options");
    sspan_opts.add_options()("search_span_bilou", "switch to (internal) BILOU encoding instead of BIO encoding");
    sch.add_program_options(vm, sspan_opts);

    if (vm.count("search_span_bilou")) {
      cerr << "switching to BILOU encoding for sequence span labeling" << endl;
      my_task_data->encoding = BILOU;
      num_actions = num_actions * 2 - 1;
    } else
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

  void setup(Search::search& sch, vector<example*>& ec) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    if (my_task_data->encoding == BILOU)
      convert_bio_to_bilou(ec);
  }

  void takedown(Search::search& sch, vector<example*>& ec) {
    task_data * my_task_data = sch.get_task_data<task_data>();

    if (my_task_data->encoding == BILOU)
      for (size_t n=0; n<ec.size(); n++) {
        MULTICLASS::multiclass ylab = ec[n]->l.multi;
        ylab.label = bilou_to_bio(ylab.label);
      }
  }
  
  void run(Search::search& sch, vector<example*>& ec) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    action last_prediction = 1;
    v_array<action> * y_allowed = &(my_task_data->allowed_actions);
    
    for (size_t i=0; i<ec.size(); i++) {
      action oracle = ec[i]->l.multi.label;
      size_t len = y_allowed->size();
      Search::predictor P(sch, (ptag)i+1);
      if (my_task_data->encoding == BIO) {
        if      (last_prediction == 1)       P.set_allowed(y_allowed->begin, len-1);
        else if (last_prediction % 2 == 0) { (*y_allowed)[len-1] = last_prediction+1; P.set_allowed(*y_allowed); }
        else                               { (*y_allowed)[len-1] = last_prediction;   P.set_allowed(*y_allowed); }
        if ((oracle > 1) && (oracle % 2 == 1) && (last_prediction != oracle) && (last_prediction != oracle-1))
          oracle = 1; // if we are supposed to I-X, but last wasn't B-X or I-X, then say O
      } else if (my_task_data->encoding == BILOU) {
        if ((last_prediction == 1) || ((last_prediction-2) % 4 == 0) || ((last_prediction-2) % 4 == 3)) { // O or unit-X or last-X
          P.set_allowed(my_task_data->allowed_actions);
          // we cannot allow in-X or last-X next
          if ((oracle > 1) && (((oracle-2) % 4 == 2) || ((oracle-2) % 4 == 3)))
            oracle = 1;
        } else { // begin-X or in-X
          action other = ((last_prediction-2) % 4 == 1) ? (last_prediction+2) : last_prediction;
          P.set_allowed(last_prediction+1);
          P.add_allowed(other);
          if ((oracle != last_prediction+1) && (oracle != other))
            oracle = other;
        }
      }
      last_prediction = P.set_input(*ec[i]).set_condition_range((ptag)i, sch.get_history_length(), 'p').set_oracle(oracle).predict();
      
      action printed_prediction = (my_task_data->encoding == BIO) ? last_prediction : bilou_to_bio(last_prediction);
      
      if (sch.output().good())
        sch.output() << printed_prediction << ' ';
    }
  }
}

namespace ArgmaxTask {
  struct task_data {
    float false_negative_cost;
    float negative_weight;
    bool predict_max;
  };

  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data* my_task_data = new task_data();
    
    po::options_description argmax_opts("argmax options");
    argmax_opts.add_options()
      ("cost", po::value<float>(&(my_task_data->false_negative_cost))->default_value(10.0), "False Negative Cost")
      ("negative_weight", po::value<float>(&(my_task_data->negative_weight))->default_value(1), "Relative weight of negative examples")
      ("max", "Disable structure: just predict the max");
    sch.add_program_options(vm, argmax_opts);

    my_task_data->predict_max = vm.count("max") > 0;

    sch.set_task_data(my_task_data);

    if (my_task_data->predict_max)
      sch.set_options( Search::EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
    else
      sch.set_options( Search::AUTO_CONDITION_FEATURES |    // automatically add history features to our examples, please
                       Search::EXAMPLES_DONT_CHANGE );   // we don't do any internal example munging
  }

  void run(Search::search& sch, vector<example*>& ec) {
    task_data * my_task_data = sch.get_task_data<task_data>();
    uint32_t max_prediction = 1;
    uint32_t max_label = 1;

    for(size_t i = 0; i < ec.size(); i++)
      max_label = max(ec[i]->l.multi.label, max_label);
        
    for (ptag i=0; i<ec.size(); i++) {
      // labels should be 1 or 2, and our output is MAX of all predicted values
      uint32_t oracle = my_task_data->predict_max ? max_label : ec[i]->l.multi.label;
      uint32_t prediction = sch.predict(*ec[i], i+1, &oracle, 1, &i, "p");

      max_prediction = max(prediction, max_prediction);
    }
    float loss = 0.;
    if (max_label > max_prediction)
      loss = my_task_data->false_negative_cost / my_task_data->negative_weight;
    else if (max_prediction > max_label)
      loss = 1.;
    sch.loss(loss);

    if (sch.output().good())
      sch.output() << max_prediction;
  }
}


namespace SequenceTask_DemoLDF {  // this is just to debug/show off how to do LDF
  namespace CS=COST_SENSITIVE;
  struct task_data {
    example* ldf_examples;
    size_t   num_actions;
  };
  
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    CS::wclass default_wclass = { 0., 0, 0., 0. };

    example* ldf_examples = alloc_examples(sizeof(CS::label), num_actions);
    for (size_t a=0; a<num_actions; a++) {
      CS::label& lab = ldf_examples[a].l.cs;
      CS::cs_label.default_label(&lab);
      lab.costs.push_back(default_wclass);
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
  void my_update_example_indicies(Search::search& sch, bool audit, example* ec, uint32_t mult_amount, uint32_t plus_amount) {
    size_t ss = sch.get_stride_shift();
    for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++)
      for (feature* f = ec->atomics[*i].begin; f != ec->atomics[*i].end; ++f)
        f->weight_index = (((f->weight_index>>ss) * mult_amount) + plus_amount)<<ss;
    if (audit)
      for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) 
        if (ec->audit_features[*i].begin != ec->audit_features[*i].end)
          for (audit_data *f = ec->audit_features[*i].begin; f != ec->audit_features[*i].end; ++f)
            f->weight_index = (((f->weight_index>>ss) * mult_amount) + plus_amount)<<ss;
  }
  
  void run(Search::search& sch, vector<example*>& ec) {
    task_data *data = sch.get_task_data<task_data>();
    for (ptag i=0; i<ec.size(); i++) {
      for (size_t a=0; a<data->num_actions; a++) {
        if (sch.predictNeedsExample()) { // we can skip this work if `predict` won't actually use the example data
          VW::copy_example_data(false, &data->ldf_examples[a], ec[i]);  // copy but leave label alone!
          // now, offset it appropriately for the action id
          my_update_example_indicies(sch, true, &data->ldf_examples[a], 28904713, 4832917 * (uint32_t)a);
        }

        // regardless of whether the example is needed or not, the class info is needed
        CS::label& lab = data->ldf_examples[a].l.cs;
        // need to tell search what the action id is, so that it can add history features correctly!
        lab.costs[0].x = 0.;
        lab.costs[0].class_index = (uint32_t)a+1;
        lab.costs[0].partial_prediction = 0.;
        lab.costs[0].wap_value = 0.;
      }

      action oracle  = ec[i]->l.multi.label - 1;
      action pred_id = Search::predictor(sch, i+1).set_input(data->ldf_examples, data->num_actions).set_oracle(oracle).set_condition_range(i, sch.get_history_length(), 'p').predict();
      action prediction = pred_id + 1;  // or ldf_examples[pred_id]->ld.costs[0].weight_index
      
      if (sch.output().good())
        sch.output() << prediction << ' ';
    }
  }
}
