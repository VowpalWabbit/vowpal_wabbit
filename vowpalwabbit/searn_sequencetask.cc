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

      MULTICLASS::mc_label* y = (MULTICLASS::mc_label*)ec[i]->ld;
      size_t prediction = srn.predict(ec[i], NULL, y->label);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (MULTICLASS::label_is_test(y) ? '?' : y->label) << ' ';
    }
  }
}

namespace OneOfManyTask {
  using namespace Searn;

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    srn.task_data            = NULL;  // we don't need any of our own data
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = false; // we will compute our own loss
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) { }    // if we had task data, we'd want to free it here

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    bool predicted_true_yet = false;
    bool output_has_true    = false;
    for (size_t i=0; i<len; i++) {
      MULTICLASS::mc_label* y = (MULTICLASS::mc_label*)ec[i]->ld;
      if (y->label == 2) output_has_true = true;
    }
        
    for (size_t i=0; i<len; i++) {
      // labels should be 1 or 2, and our output is MAX of all predicted values
      srn.snapshot(i, 1, &i, sizeof(i), true); //save state for optimization
      srn.snapshot(i, 2, &predicted_true_yet, sizeof(predicted_true_yet), false);  // not used for prediction

      MULTICLASS::mc_label* y = (MULTICLASS::mc_label*)ec[i]->ld;
      size_t prediction = srn.predict(ec[i], NULL, y->label);

      float cur_loss = 0.;
      if (prediction == 2) { // we predicted "yes"
        if (!predicted_true_yet) { // and this is the first time
          if (output_has_true) cur_loss = 0.;
          else cur_loss = 1.;
        } else { // we've predicted true earlier
          if (output_has_true) cur_loss = 0.;
          else cur_loss = 1.; // TODO: should this be zero? i.e., should we not get repeatedly punished?
        }
        predicted_true_yet = true;
      } else { // we predicted "no"
        if (!predicted_true_yet) { // no predictions of true at all
          if ((i == len-1) && output_has_true)
            cur_loss = 1.;  // totally hosed
        } else { // we've predicted true in the past
          // no loss
        }
      }
      srn.declare_loss(1, cur_loss);

      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (MULTICLASS::label_is_test(y) ? '?' : y->label) << ' ';
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
//     1     -> 1; 2, 6, 8, ...; 3, 7, 9, ...         out to { out, unit-Y, begin-Y }       1
//     m%4=0 -> 1; 2, 6, 8, ..., 3, 7, 9, ...         unit-X to { out, unit-Y, begin-Y }    2, 6, 10, 14, ...
//     m%4=1 -> m+1, m+2                              begin-X to { in-X, last-X }           3, 7, 11, 15, ...
//     m%4=2 -> m, m+1                                in-X to { in-X, last-X }              4, 8, 12, 16, ...
//     m%4=3 -> 1; 2, 6, 8, ...; 3, 7, 9, ...         last-X to { out, unit-Y, begin-Y }     5, 9, 13, 17, ...

  using namespace Searn;

  void convert_bio_to_bilou(example**ec, size_t len) {
    for (size_t n=0; n<len; n++) {
      MULTICLASS::mc_label* ylab = (MULTICLASS::mc_label*)ec[n]->ld;
      uint32_t y = ylab->label;
      uint32_t nexty = (n == len-1) ? 0 : ((MULTICLASS::mc_label*)ec[n+1]->ld)->label;
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

  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    task_data * my_task_data = new task_data();

    po::options_description desc("Searn options");
    desc.add_options()("searn_bilou", "switch to (internal) BILOU encoding instead of BIO encoding");
    po::options_description add_desc_file("Searn options only available in regressor file");
    add_desc_file.add_options()("searn_trained_nb_policies", po::value<size_t>(), "the number of trained policies in the regressor file");

    po::options_description desc_file;
    desc_file.add(desc).add(add_desc_file);

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    po::parsed_options parsed_file = po::command_line_parser(srn.all->options_from_file_argc, srn.all->options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc_file).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);
    if (vm.count("searn_bilou") || vm_file.count("searn_bilou")) {
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
    
    srn.task_data            = my_task_data;
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won'td eclare_loss
    srn.examples_dont_change = true;  // we don't do any internal example munging
  }

  void finish(searn& srn) {
    task_data * my_task_data = (task_data*)srn.task_data;
    my_task_data->y_allowed.delete_v();
    my_task_data->only_two_allowed.delete_v();
    delete my_task_data;
  }

  void structured_predict(searn& srn, example**ec, size_t len, stringstream*output_ss, stringstream*truth_ss) {
    task_data * my_task_data = (task_data*)srn.task_data;
    uint32_t last_prediction = 1;
    v_array<uint32_t> * y_allowed = &(my_task_data->y_allowed);

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      convert_bio_to_bilou(ec, len);
    
    for (size_t i=0; i<len; i++) {
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
      MULTICLASS::mc_label* y = (MULTICLASS::mc_label*)ec[i]->ld;
      last_prediction = srn.predict(ec[i], y_allowed, y->label);

      uint32_t printed_prediction = (my_task_data->encoding == BIO) ? last_prediction : bilou_to_bio(last_prediction);
      uint32_t printed_truth      = (my_task_data->encoding == BIO) ? y->label        : bilou_to_bio(y->label);
      
      if (output_ss) (*output_ss) << printed_prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (MULTICLASS::label_is_test(y) ? '?' : printed_truth) << ' ';
    }

    if (my_task_data->encoding == BILOU)  // TODO: move this out of here!
      for (size_t n=0; n<len; n++) {
        MULTICLASS::mc_label* ylab = (MULTICLASS::mc_label*)ec[n]->ld;
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
  
  void initialize(searn& srn, size_t& num_actions, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file) {
    COST_SENSITIVE::wclass default_wclass = { 0., 0, 0., 0. };

    example* ldf_examples = alloc_examples(sizeof(COST_SENSITIVE::label), num_actions);
    for (size_t a=0; a<num_actions; a++) {
      COST_SENSITIVE::label* lab = (COST_SENSITIVE::label*)ldf_examples[a].ld;
      lab->costs.push_back(default_wclass);
    }

    task_data* data = (task_data*)calloc_or_die(1, sizeof(task_data));
    data->ldf_examples = ldf_examples;
    data->num_actions  = num_actions;
    
    srn.task_data            = data;
    srn.auto_history         = true;  // automatically add history features to our examples, please
    srn.auto_hamming_loss    = true;  // please just use hamming loss on individual predictions -- we won't declare_loss
    srn.examples_dont_change = false; // we do internal example munging -- we use the same memory space (data->ldf_examples) for everything
    srn.is_ldf               = true;  // we generate ldf examples
  }

  void finish(searn& srn) {
    task_data *data = (task_data*)srn.task_data;
    for (size_t a=0; a<data->num_actions; a++)
      dealloc_example(COST_SENSITIVE::delete_label, data->ldf_examples[a]);
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
        COST_SENSITIVE::label* lab = (COST_SENSITIVE::label*)data->ldf_examples[a].ld;
        lab->costs[0].x = 0.;
        lab->costs[0].weight_index = (uint32_t)a+1;
        lab->costs[0].partial_prediction = 0.;
        lab->costs[0].wap_value = 0.;
      }
      
      MULTICLASS::mc_label* y = (MULTICLASS::mc_label*)ec[i]->ld;
      size_t pred_id = srn.predict(data->ldf_examples, data->num_actions, NULL, y->label - 1);
      size_t prediction = pred_id + 1;  // or ldf_examples[pred_id]->ld.costs[0].weight_index
      
      if (output_ss) (*output_ss) << prediction << ' ';
      if (truth_ss ) (*truth_ss ) << (MULTICLASS::label_is_test(y) ? '?' : y->label) << ' ';
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
