/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/
#ifndef SEARCH_H
#define SEARCH_H

#include "global_data.h"

#define cdbg clog
//#undef cdbg
//#define cdbg if (1) {} else clog
// uncomment the previous two lines if you want loads of debug output :)

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

