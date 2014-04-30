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
  extern uint32_t OPT_AUTO_HISTORY, OPT_AUTO_HAMMING_LOSS, OPT_EXAMPLES_DONT_CHANGE, OPT_IS_LDF;


  struct searn {
    // functions that you will call

    inline uint32_t predict(example* ecs, size_t ec_len, v_array<uint32_t>* yallowed, v_array<uint32_t>* ystar) // for LDF
    { return this->predict_f(this->priv, ecs, ec_len, yallowed, ystar, false); }

    inline uint32_t predict(example* ecs, size_t ec_len, v_array<uint32_t>* yallowed, uint32_t one_ystar) // for LDF
    { if (one_ystar == (uint32_t)-1) // test example
        return this->predict_f(this->priv, ecs, ec_len, yallowed, NULL, false);
      else
        return this->predict_f(this->priv, ecs, ec_len, yallowed, (v_array<uint32_t>*)&one_ystar, true);
    }

    inline uint32_t predict(example* ec, v_array<uint32_t>* yallowed, v_array<uint32_t>* ystar) // for not LDF
    { return this->predict_f(this->priv, ec, 0, yallowed, ystar, false); }

    inline uint32_t predict(example* ec, v_array<uint32_t>* yallowed, uint32_t one_ystar) // for not LDF
    { if (one_ystar == (uint32_t)-1) // test example
        return this->predict_f(this->priv, ec, 0, yallowed, NULL, false);
      else
        return this->predict_f(this->priv, ec, 0, yallowed, (v_array<uint32_t>*)&one_ystar, true);
    }
    
    inline void     declare_loss(size_t predictions_since_last, float incr_loss)
    { return this->declare_loss_f(this->priv, predictions_since_last, incr_loss); }

    inline void     snapshot(size_t index, size_t tag, void* data_ptr, size_t sizeof_data, bool used_for_prediction)
    { return this->snapshot_f(this->priv, index, tag, data_ptr, sizeof_data, used_for_prediction); }

    inline bool          should_generate_output() { return this->should_generate_output_f(this->priv); }
    inline stringstream& output()                 { return this->output_stringstream_f(this->priv); }
    
    inline void  set_task_data(void*data)   { this->set_task_data_f(this->priv, data); }
    inline void* get_task_data()            { return this->get_task_data_f(this->priv); }
    inline void  set_options(uint32_t opts) { this->set_options_f(this->priv, opts); }

    // structure that you must set, and any associated data you want to store
    searn_task*    task;
    searn_private* priv;

    // functions that you should never call directly
    uint32_t (*predict_f)(searn_private*,example*,size_t,v_array<uint32_t>*,v_array<uint32_t>*,bool);
    void     (*declare_loss_f)(searn_private*,size_t,float);   // <0 means it was a test example!
    void     (*snapshot_f)(searn_private*,size_t,size_t,void*,size_t,bool);
    bool     (*should_generate_output_f)(searn_private*);
    stringstream& (*output_stringstream_f)(searn_private*);
    void     (*set_task_data_f)(searn_private*,void*data);
    void*    (*get_task_data_f)(searn_private*);
    void     (*set_options_f)(searn_private*,uint32_t opts);
  };

  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string);
  void check_option(bool& ret, vw&all, po::variables_map& vm, po::variables_map& vm_file, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string);
  bool string_equal(string a, string b);
  bool float_equal(float a, float b);
  bool uint32_equal(uint32_t a, uint32_t b);
  bool size_equal(size_t a, size_t b);

  struct searn_task {
    void (*initialize)(searn&,size_t&,std::vector<std::string>&, po::variables_map&, po::variables_map&);
    void (*finish)(searn&);
    void (*structured_predict)(searn&, example**,size_t);
  };

  LEARNER::learner* setup(vw&, std::vector<std::string>&, po::variables_map&, po::variables_map&);
  void searn_finish(void*);
  void searn_drive(void*);
  void searn_learn(void*,example*);
}

#endif
