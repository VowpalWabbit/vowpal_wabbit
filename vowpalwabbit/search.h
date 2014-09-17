/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#ifndef SEARCH_H
#define SEARCH_H

#include "global_data.h"

#define cdbg clog
#undef cdbg
#define cdbg if (1) {} else clog
// comment the previous two lines if you want loads of debug output :)

typedef uint32_t    action;
typedef uint32_t    ptag;

namespace Search {
  struct search_private;
  struct search_task;

  extern uint32_t AUTO_CONDITION_FEATURES, AUTO_HAMMING_LOSS, EXAMPLES_DONT_CHANGE, IS_LDF;

  struct search {
    // INTERFACE
    // for managing task-specific data that you want on the heap:
    template<class T> void  set_task_data(T*data)           { task_data = data; }
    template<class T> T*    get_task_data()                 { return (T*)task_data; }

    // for setting programmatic options during initialization
    // this should be an or ("|") of AUTO_CONDITION_FEATURES, etc.
    void set_options(uint32_t opts);

    // for explicitly declaring a loss incrementally
    void loss(float incr_loss);

    // make a prediction with lots of options
    //    TODO: describe options
    action predict(        example& ec
                   ,       ptag     my_tag
                   , const action*  oracle_actions
                   ,       size_t   oracle_actions_cnt   = 1
                   , const ptag*    condition_on         = NULL
                   , const char*    condition_on_names   = NULL   // strlen(condition_on_names) should == |condition_on|
                   , const action*  allowed_actions      = NULL
                   ,       size_t   allowed_actions_cnt  = 0
                   ,       size_t   learner_id           = 0
                   );

    // make an LDF prediction with lots of options
    //    TODO: describe options
    action predictLDF(        example* ecs
                      ,       size_t ec_cnt
                      ,       ptag     my_tag
                      , const action*  oracle_actions
                      ,       size_t   oracle_actions_cnt   = 1
                      , const ptag*    condition_on         = NULL
                      , const char*    condition_on_names   = NULL
                      ,       size_t   learner_id           = 0
                      );
    
    // where you should write output
    std::stringstream& output();

    // set the number of learners
    void set_num_learners(size_t num_learners);

    // internal data that you don't get to see!
    search_private* priv;
    void*           task_data;  // your task data!
  };

  // for defining new tasks, you must fill out a search_task
  struct search_task {
    // required
    const char* task_name;
    void (*run)(search&, std::vector<example*>&);

    // optional
    void (*initialize)(search&,size_t&, po::variables_map&);
    void (*finish)(search&);
    void (*run_setup)(search&, std::vector<example*>&);
    void (*run_takedown)(search&, std::vector<example*>&);
  };

  // to make calls to "predict" (and "predictLDF") cleaner when you
  // want to use crazy combinations of arguments
  class predictor {
    public:
    predictor(search& sch) : my_tag(0), sch(sch) {}

    predictor& set_input(example&input_example) {
      is_ldf = false;
      ec = &input_example;
      ec_cnt = 1;
      return *this;
    }

    predictor& set_input(example*input_example, size_t input_length) {
      is_ldf = true;
      ec = input_example;
      ec_cnt = input_length;
      return *this;
    }

    // different ways of adding to the list of oracle actions
    predictor& add_oracle(action a) { oracle_actions.push_back(a); return *this; }
    predictor& add_oracle(action*a, size_t action_count) {
      if (oracle_actions.size() > 0)
        push_many<action>(oracle_actions, a, action_count);
      else {
        oracle_actions.begin = a;
        oracle_actions.end   = a + action_count;
        oracle_actions.end_array = a + action_count;
      }
      return *this;
    }
    predictor& add_oracle(v_array<action> a) { add_oracle(a.begin, a.size()); return *this; }

    predictor& set_oracle(action a) { oracle_actions.erase(); return add_oracle(a); }
    predictor& set_oracle(action*a, size_t action_count) { oracle_actions.erase(); return add_oracle(a, action_count); }
    predictor& set_oracle(v_array<action> a) { oracle_actions.erase(); return add_oracle(a); }
    
    // different ways of adding allowed actions
    predictor& add_allowed(action a) { allowed_actions.push_back(a); return *this; }
    predictor& add_allowed(action*a, size_t action_count) {
      if (allowed_actions.size() > 0)
        push_many<action>(allowed_actions, a, action_count);
      else {
        allowed_actions.begin = a;
        allowed_actions.end   = a + action_count;
        allowed_actions.end_array = allowed_actions.end;
      }
      return *this;
    }
    predictor& add_allowed(v_array<action> a) { add_allowed(a.begin, a.size()); return *this; }
    
    predictor& set_allowed(action a) { allowed_actions.erase(); return add_allowed(a); }
    predictor& set_allowed(action*a, size_t action_count) { allowed_actions.erase(); return add_allowed(a, action_count); }
    predictor& set_allowed(v_array<action> a) { allowed_actions.erase(); return add_allowed(a); }

    // different ways of adding conditioning
    predictor& add_condition(ptag tag, char name) { condition_on_tags.push_back(tag); condition_on_names.push_back(name); return *this; }
    predictor& set_condition(ptag tag, char name) { condition_on_tags.erase(); condition_on_names.erase(); return add_condition(tag, name); }

    // set learner id
    predictor& set_learner_id(size_t id) { learner_id = id; return *this; }

    // set tag
    predictor& set_tag(ptag tag) { my_tag = tag; return *this; }
    
    action predict() {
      const action* orA = oracle_actions.size() == 0 ? NULL : oracle_actions.begin;
      const ptag*   cOn = condition_on_names.size() == 0 ? NULL : condition_on_tags.begin;
      const char*   cNa = NULL;
      if (condition_on_names.size() > 0) {
        condition_on_names.push_back((char)0);  // null terminate
        cNa = condition_on_names.begin;
      }
      const action* alA = (allowed_actions.size() == 0) ? NULL : allowed_actions.begin;

      if (is_ldf)
        return sch.predictLDF(ec, ec_cnt, my_tag, orA, oracle_actions.size(), cOn, cNa, learner_id);
      else
        return sch.predict(*ec, my_tag, orA, oracle_actions.size(), cOn, cNa, alA, allowed_actions.size(), learner_id);

      if (condition_on_names.size() > 0)
        condition_on_names.pop();  // un-null-terminate
    }
    
    private:
    bool is_ldf;
    ptag my_tag;
    example* ec;
    size_t ec_cnt;
    v_array<action> oracle_actions;
    v_array<ptag> condition_on_tags;
    v_array<char> condition_on_names;
    v_array<action> allowed_actions;
    size_t learner_id;
    search&sch;
  };
  
  // some helper functions you might find helpful
  template<class T> void check_option(T& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string);
  void check_option(bool& ret, vw&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline, const char* mismatch_error_string);
  bool string_equal(string a, string b);
  bool float_equal(float a, float b);
  bool uint32_equal(uint32_t a, uint32_t b);
  bool size_equal(size_t a, size_t b);

  // our interface within VW
  LEARNER::learner* setup(vw&, po::variables_map&);
  void search_finish(void*);
  void search_drive(void*);
  void search_learn(void*,example*);  
}



#endif

