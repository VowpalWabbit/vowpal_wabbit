/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SEARN_H
#define SEARN_H

#include <stdio.h>
#include "parse_args.h"
#include "oaa.h"
#include "parse_primitives.h"
#include "v_hashmap.h"
#include "csoaa.h"
#include <time.h>

#define clog_print_audit_features(ec,reg) { print_audit_features(reg, ec); }

typedef uint32_t* history;

namespace SearnUtil
{
  struct history_info {
    size_t length;          // was history_length, must be >= features
    bool   bigrams;         // was sequence_bigrams
    size_t features;        // was sequence_features
    bool   bigram_features; // was sequence_bigram_features
  };
  void default_info(history_info*);


  void* calloc_or_die(size_t, size_t);
  void free_it(void*);

  int  random_policy(uint64_t, float, bool, int, bool, bool);

  void add_policy_offset(vw&, example*, uint32_t, uint32_t);
  void remove_policy_offset(vw&, example*, uint32_t, uint32_t);
 
  void add_history_to_example(vw&, history_info&, example*, history);
  void remove_history_from_example(vw&, history_info&, example*);

  size_t predict_with_history(vw&vw, example*ec, v_array<uint32_t>*ystar, history_info &hinfo, size_t*history);
}      

namespace Searn {
  using namespace SearnUtil;

  struct snapshot_item {
    size_t index;
    size_t tag;
    void  *data_ptr;
    size_t data_size;  // sizeof(data_ptr)
    size_t pred_step;  // srn->t when snapshot is made
  };
  
  struct searn_task;

  struct searn {
    // functions that you will call

    inline uint32_t predict(example** ecs, size_t ec_len, v_array<uint32_t>* yallowed, v_array<uint32_t>* ystar) // for LDF
    { return this->predict_f(*this->all, *this->base_learner, ecs, ec_len, yallowed, ystar, false); }

    inline uint32_t predict(example* ec, v_array<uint32_t>* yallowed, v_array<uint32_t>* ystar) // for not LDF
    { return this->predict_f(*this->all, *this->base_learner, &ec, 0, yallowed, ystar, false); }

    inline uint32_t predict(example* ec, v_array<uint32_t>* yallowed, OAA::mc_label* one_ystar) // for not LDF
    { if (OAA::label_is_test(one_ystar))
        return this->predict_f(*this->all, *this->base_learner, &ec, 0, yallowed, NULL, false);
      else
        return this->predict_f(*this->all, *this->base_learner, &ec, 0, yallowed, (v_array<uint32_t>*)&one_ystar->label, true);
    }
        //    { OAA::label_to_array(one_ystar, this->predict_ystar);
        //      return this->predict_f(*this->all, *this->base_learner, &ec, 0, yallowed, &this->predict_ystar, false); }
      
    
    inline void     declare_loss(size_t predictions_since_last, float incr_loss)
    { return this->declare_loss_f(*this->all, predictions_since_last, incr_loss); }

    inline void     snapshot(size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction)
    { return this->snapshot_f(*this->all, index, tag, data_ptr, sizeof_data, used_for_prediction); }

    // structure that you must set, and any associated data you want to store
    searn_task* task;
    void* task_data;
    bool auto_history;          // do you want us to automatically add history features?
    bool auto_hamming_loss;     // if you're just optimizing hamming loss, we can do it for you!
    bool examples_dont_change;  // set to true if you don't do any internal example munging

    // data that you should not look at.  ever.
    uint32_t (*predict_f)(vw&,learner&,example**,size_t,v_array<uint32_t>*,v_array<uint32_t>*,bool);
    void     (*declare_loss_f)(vw&,size_t,float);   // <0 means it was a test example!
    void     (*snapshot_f)(vw&,size_t,size_t,void*,size_t,bool);
    
    v_array<uint32_t> predict_ystar;
    size_t A;             // total number of actions, [1..A]; 0 means ldf
    char state;           // current state of learning
    size_t learn_t;       // when LEARN, this is the t at which we're varying a
    uint32_t learn_a;     //   and this is the a we're varying it to
    size_t snapshot_is_equivalent_to_t;   // if we've finished snapshotting and are equiv up to this time step, then we can fast forward from there
    bool snapshot_could_match;
    size_t snapshot_last_found_pos;
    v_array<snapshot_item> snapshot_data;
    v_array<uint32_t> train_action;  // which actions did we actually take in the train (or test) pass?
    v_array< void* > train_labels;  // which labels are valid at any given time
    v_array<uint32_t> rollout_action; // for auto_history, we need a space other than train_action for rollouts
    history_info hinfo;   // default history info for auto-history
    string *neighbor_features_string;
    v_array<int32_t> neighbor_features; // ugly encoding of neighbor feature requirements
    
    bool should_produce_string;
    stringstream *pred_string;
    stringstream *truth_string;
    bool printed_output_header;

    size_t t;              // the current time step
    size_t T;              // the length of the (training) trajectory
    size_t loss_last_step; // at what time step did they last declare their loss?
    float  test_loss;      // total test loss for this example
    float  train_loss;     // total training loss for this example
    float  learn_loss;     // total loss for this "varied" example

    v_array<float> learn_losses;   // losses for all (valid) actions at learn_t
    example** learn_example_copy; // copy of example(s) at learn_t
    size_t learn_example_len;     // number of example(s) at learn_t

    float  beta;                  // interpolation rate
    bool   allow_current_policy;  // should the current policy be used for training? true for dagger
    bool   rollout_oracle; //if true then rollout are performed using oracle instead (optimal approximation discussed in searn's paper). this should be set to true for dagger
    bool   adaptive_beta; //used to implement dagger through searn. if true, beta = 1-(1-alpha)^n after n updates, and policy is mixed with oracle as \pi' = (1-beta)\pi^* + beta \pi
    bool   rollout_all_actions;   // by default we rollout all actions. This is set to false when searn is used with a contextual bandit base learner, where we rollout only one sampled action
    float  alpha; //parameter used to adapt beta for dagger (see above comment), should be in (0,1)
    uint32_t current_policy;      // what policy are we training right now?
    float gamma;                  // for dagger

    size_t num_features;
    uint32_t total_number_of_policies;
    bool do_snapshot;
    bool do_fastforward;
    float subsample_timesteps;

    size_t read_example_last_id;
    size_t passes_since_new_policy;
    size_t read_example_last_pass;
    size_t total_examples_generated;
    size_t total_predictions_made;

    bool hit_new_pass;
    
    size_t passes_per_policy;

    v_array<example*> ec_seq;

    learner* base_learner;
    vw* all;
    void* valid_labels;
    clock_t start_clock_time;
    
    example*empty_example;
  };

  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string);
  void check_option(bool& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string);
  bool string_equal(string a, string b);
  bool float_equal(float a, float b);
  bool uint32_equal(uint32_t a, uint32_t b);
  bool size_equal(size_t a, size_t b);
  void setup_searn_options(po::options_description& desc, vw&vw, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file);


  struct searn_task {
    void (*initialize)(searn&,size_t&,std::vector<std::string>&, po::variables_map&, po::variables_map&);
    void (*finish)(searn&);
    void (*structured_predict)(searn&, example**,size_t,stringstream*,stringstream*);
  };

  learner* setup(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void searn_finish(void*);
  void searn_drive(void*);
  void searn_learn(void*,example*);
}

#endif
