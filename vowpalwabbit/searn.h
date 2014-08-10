/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#ifndef SEARN_H
#define SEARN_H

#include <stdio.h>
#include "parse_args.h"
#include "parse_primitives.h"
#include "v_hashmap.h"
#include "cost_sensitive.h"
#include <time.h>

#define clog_print_audit_features(ec,reg) { print_audit_features(reg, ec); }
#define MAX_BRANCHING_FACTOR 128

#define cdbg clog
#undef cdbg
#define cdbg if (1) {} else clog

namespace Searn {

  struct searn_private;
  struct searn_task;

  // options:
  extern uint32_t AUTO_HISTORY, AUTO_HAMMING_LOSS, EXAMPLES_DONT_CHANGE, IS_LDF;

  struct searn {
    // INTERFACE
    // for managing task-specific data that you want on the heap:
    template<class T> void  set_task_data(T*data)           { task_data = data; }
    template<class T> T*    get_task_data()                 { return (T*)task_data; }

    // for setting programmatic options during initialization
    void set_options(uint32_t opts);

    // for snapshotting your algorithm's state
    void snapshot(size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction);

    // for explicitly declaring your loss
    void loss(float incr_loss, size_t predictions_since_last=1);

    // for making predictions in regular (non-LDF) mode:
    uint32_t predict(example* ec, uint32_t       one_ystar, v_array<uint32_t>* yallowed=NULL, size_t learner_id=0); // if there is a single oracle action
    uint32_t predict(example* ec, v_array<uint32_t>* ystar, v_array<uint32_t>* yallowed=NULL, size_t learner_id=0); // if there are multiple oracle actions

    // for making predictions in LDF mode:
    uint32_t predictLDF(example* ecs, size_t ec_len, v_array<uint32_t>* ystar, v_array<uint32_t>* yallowed=NULL, size_t learner_id=0); // if there is a single oracle action
    uint32_t predictLDF(example* ecs, size_t ec_len, uint32_t       one_ystar, v_array<uint32_t>* yallowed=NULL, size_t learner_id=0); // if there is are multiple oracle action

    // for generating output (check to see if output().good() before attempting to write!)
    stringstream& output();

    // set number of learners
    void set_num_learners(size_t num_learner);
    
    // internal data
    searn_task*    task;
    vw* all;
    searn_private* priv;
    void*          task_data;
  };

  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string);
  void check_option(bool& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string);
  bool string_equal(string a, string b);
  bool float_equal(float a, float b);
  bool uint32_equal(uint32_t a, uint32_t b);
  bool size_equal(size_t a, size_t b);

  struct searn_task {
    const char* task_name;
    void (*initialize)(searn&,size_t&, po::variables_map&);
    void (*finish)(searn&);
    void (*structured_predict)(searn&, std::vector<example*>);
  };

  LEARNER::learner* setup(vw&, po::variables_map&);
  void searn_finish(void*);
  void searn_drive(void*);
  void searn_learn(void*,example*);

  typedef uint32_t* history;

  struct history_info {
    size_t length;          // was history_length, must be >= features
    bool   bigrams;         // was sequence_bigrams
    size_t features;        // was sequence_features
    bool   bigram_features; // was sequence_bigram_features
  };
  void default_info(history_info*);

  void add_history_to_example(vw&, history_info&, example*, history, size_t);
  void remove_history_from_example(vw&, history_info&, example*);
}

#endif
