// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "vw/core/example.h"
#include "vw/core/global_data.h"

// TODO: Search is using some macro-enabled logging logic for cdbg
//       (going to clog [which in turn goes to err, with some differences])
//       We may want to create/use some macro-based loggers (which will wrap the spdlog ones)
//       to mimic this behavior.
#define cdbg std::clog
#undef cdbg
#define cdbg \
  if (1) {}  \
  else       \
    std::clog
// comment the previous two lines if you want loads of debug output :)

using action = uint32_t;
using ptag = uint32_t;

namespace Search
{
class search_private;
class search_task;

extern uint32_t AUTO_CONDITION_FEATURES, AUTO_HAMMING_LOSS, EXAMPLES_DONT_CHANGE, IS_LDF, NO_CACHING, ACTION_COSTS;

class search;

class BaseTask
{
public:
  BaseTask(search* _sch, VW::multi_ex& _ec) : sch(_sch), ec(_ec)
  {
    _foreach_action = nullptr;
    _post_prediction = nullptr;
    _maybe_override_prediction = nullptr;
    _with_output_string = nullptr;
    _final_run = false;
  }
  inline BaseTask& foreach_action(void (*f)(search&, size_t, float, action, bool, float))
  {
    _foreach_action = f;
    return *this;
  }
  inline BaseTask& post_prediction(void (*f)(search&, size_t, action, float))
  {
    _post_prediction = f;
    return *this;
  }
  inline BaseTask& maybe_override_prediction(bool (*f)(search&, size_t, action&, float&))
  {
    _maybe_override_prediction = f;
    return *this;
  }
  inline BaseTask& with_output_string(void (*f)(search&, std::stringstream&))
  {
    _with_output_string = f;
    return *this;
  }
  inline BaseTask& final_run()
  {
    _final_run = true;
    return *this;
  }

  void Run();

  // data
  search* sch;
  VW::multi_ex& ec;
  bool _final_run;
  void (*_foreach_action)(search&, size_t, float, action, bool, float);
  void (*_post_prediction)(search&, size_t, action, float);
  bool (*_maybe_override_prediction)(search&, size_t, action&, float&);
  void (*_with_output_string)(search&, std::stringstream&);
};

class search
{
public:  // INTERFACE
  // for managing task-specific data that you want on the heap:
  template <class T>
  void set_task_data(T* data)
  {
    task_data = std::shared_ptr<T>(data);
  }
  template <class T>
  T* get_task_data()
  {
    return static_cast<T*>(task_data.get());
  }

  // for managing metatask-specific data
  template <class T>
  void set_metatask_data(T* data)
  {
    metatask_data = std::shared_ptr<T>(data);
  }
  template <class T>
  T* get_metatask_data()
  {
    return static_cast<T*>(metatask_data.get());
  }

  // for setting programmatic options during initialization
  // this should be an or ("|") of AUTO_CONDITION_FEATURES, etc.
  void set_options(uint32_t opts);

  // change the default label parser, but you _must_ tell me how
  // to detect test examples!
  void set_label_parser(VW::label_parser& lp, bool (*is_test)(const VW::polylabel&));

  // for explicitly declaring a loss incrementally
  void loss(float incr_loss);

  // make a prediction on an example. returns the predicted action.
  // arguments:
  //   ec                    the example (features) on which to make a prediction
  //   my_tag                a tag for this prediction, so that you can explicitly
  //                           state, for future predictions, which ones depend
  //                           explicitly or implicitly on this prediction
  //   oracle_actions        an array of actions that the oracle would take
  //                           nullptr => the oracle doesn't know (is random!)
  //   oracle_actions_cnt    the length of the previous array, or 0 if it's nullptr
  //   condition_on          an array of previous (or future) predictions on which
  //                           this prediction depends. the semantics of conditioning
  //                           is that IF the predictions for all the tags in
  //                           condition_on were the same, then the prediction for
  //                           _this_ example will also be the same. i.e., same
  //                           features, etc. (also assuming same policy). if
  //                           AUTO_CONDITION_FEATURES is on, then we will automatically
  //                           add features to ec based on what you're conditioning on.
  //                           nullptr => independent prediction
  //   condition_on_names    a std::string containing the list of names of features you're
  //                           conditioning on. used explicitly for auditing, implicitly
  //                           for keeping tags separated. also, strlen(condition_on_names)
  //                           tells us how long condition_on is
  //   allowed_actions       an array of actions that are allowed at this step, or
  //                           nullptr if everything is allowed
  //   allowed_actions_cnt   the length of allowed_actions (0 if allowed_actions is null)
  //   allowed_actions_cost  if you can precompute the cost-under-rollout-by-ref for each
  //                           allowed action, and the underlying algorithm can use this
  //                           (i.e., rollout=none or rollout=mix_per_roll and we're on
  //                           a rollout-by-ref), then fill this in and rollouts will be
  //                           avoided. note: if you provide allowed_actions_cost,
  //                           then oracle_actions will be ignored (might as well pass
  //                           nullptr). if allowed_actions
  //                           is a nullptr, then allowed_actions_cost should be a vector
  //                           of length equal to the total number of actions ("A"); otherwise
  //                           it should be of length allowed_actions_cnt. only valid
  //                           if ACTION_COSTS is specified as an option.
  //   learner_id            the id for the underlying learner to use (via set_num_learners)
  action predict(VW::example& ec, ptag my_tag, const action* oracle_actions, size_t oracle_actions_cnt = 1,
      const ptag* condition_on = nullptr,
      const char* condition_on_names = nullptr  // strlen(condition_on_names) should == |condition_on|
      ,
      const action* allowed_actions = nullptr, size_t allowed_actions_cnt = 0,
      const float* allowed_actions_cost = nullptr, size_t learner_id = 0, float weight = 0.);

  // make an LDF prediction on a list of examples. arguments are identical to predict(...)
  // with the following exceptions:
  //   * ecs/ec_cnt replace ec. ecs is the list of examples the make up a single
  //     LDF example, and ec_cnt is its length
  //   * there are no more "allowed_actions" because that is implicit in the LDF
  //     example structure. additionally, allowed_actions_cost should be stored
  //     in the label structure for ecs (if ACTION_COSTS is set as an option)
  action predictLDF(VW::example* ecs, size_t ec_cnt, ptag my_tag, const action* oracle_actions,
      size_t oracle_actions_cnt = 1, const ptag* condition_on = nullptr, const char* condition_on_names = nullptr,
      size_t learner_id = 0, float weight = 0.);

  // some times during training, a call to "predict" doesn't
  // actually use the example you pass (*), and for efficiency you
  // might want to forgo the construction of examples in those
  // cases. if a call to predictNeedsExample() returns true, then
  // then any subsequent call to predict should be sure to include
  // correctly processed examples. if it returns false, you can pass
  // anything to the next call to predict.
  //
  // (*) the slight exception is for predictLDF. in this case, we
  // always need to provide some examples so that we know which
  // actions are possible. in LDF mode, if predictNeedsExample()
  // returns false, then it's okay to just provide the labels in
  // your subsequent call to predictLDF(), and skip the feature
  // values.
  bool predictNeedsExample();

  // get the value specified by --search_history_length
  uint32_t get_history_length();

  // check if the user declared ldf mode
  bool is_ldf();

  // where you should write output
  std::stringstream& output();

  // set the number of learners
  void set_num_learners(size_t num_learners);

  // get the action sequence from the test run (only run if test_only or -t or...)
  void get_test_action_sequence(std::vector<action>&);

  // get feature index mask
  uint64_t get_mask();

  // get stride_shift
  size_t get_stride_shift();

  // pretty print a label
  std::string pretty_label(action a);

  // for meta-tasks:
  BaseTask base_task(VW::multi_ex& ec) { return BaseTask(this, ec); }

  // internal data that you don't get to see!
  search_private* priv = nullptr;
  std::shared_ptr<void> task_data = nullptr;      // your task data!
  std::shared_ptr<void> metatask_data = nullptr;  // your metatask data!
  const char* task_name = nullptr;
  const char* metatask_name = nullptr;

  VW::workspace& get_vw_pointer_unsafe();  // although you should rarely need this, some times you need a pointer to the
                                           // vw data structure :(
  void set_force_oracle(bool force);       // if the library wants to force search to use the oracle, set this to true
  search();
  ~search();
};

// for defining new tasks, you must fill out a search_task
class search_task
{
public:  // required
  const char* task_name;
  void (*run)(search&, VW::multi_ex&);

  // optional
  void (*initialize)(search&, size_t&, VW::config::options_i&);
  void (*finish)(search&);
  void (*run_setup)(search&, VW::multi_ex&);
  void (*run_takedown)(search&, VW::multi_ex&);
};

class search_metatask
{
public:  // required
  const char* metatask_name;
  void (*run)(search&, VW::multi_ex&);

  // optional
  void (*initialize)(search&, size_t&, VW::config::options_i&);
  void (*finish)(search&);
  void (*run_setup)(search&, VW::multi_ex&);
  void (*run_takedown)(search&, VW::multi_ex&);
};

// to make calls to "predict" (and "predictLDF") cleaner when you
// want to use crazy combinations of arguments
class predictor
{
public:
  predictor(search& sch, ptag my_tag);

  // tell the predictor what to use as input. a single example input
  // means non-LDF mode; an array of inputs means LDF mode
  predictor& set_input(VW::example& input_example);
  predictor& set_input(
      VW::example* input_example, size_t input_length);  // if you're lucky and have an array of examples

  // the following is mostly to make life manageable for the Python interface
  void set_input_length(size_t input_length);                  // declare that we have an input_length-long LDF example
  void set_input_at(size_t posn, VW::example& input_example);  // set the corresponding input (*after* set_input_length)

  // different ways of adding to the list of oracle actions. you can
  // either add_ or set_; setting erases previous actions. these
  // functions attempt to allocate as little memory as possible, so if
  // you pass a v_array or an action*, unless you later add something
  // else, we'll just store a pointer to your memory. this means that
  // you probably shouldn't change the data there, or free that pointer,
  // between calling add/set_oracle and calling predict()
  predictor& erase_oracles();

  predictor& reset();

  predictor& add_oracle(action a);
  predictor& add_oracle(action* a, size_t action_count);
  predictor& add_oracle(VW::v_array<action>& a);

  predictor& set_oracle(action a);
  predictor& set_oracle(action* a, size_t action_count);
  predictor& set_oracle(VW::v_array<action>& a);

  predictor& set_weight(float w);

  // same as add/set_oracle but for allowed actions
  predictor& erase_alloweds();

  predictor& add_allowed(action a);
  predictor& add_allowed(action* a, size_t action_count);
  predictor& add_allowed(VW::v_array<action>& a);

  predictor& set_allowed(action a);
  predictor& set_allowed(action* a, size_t action_count);
  predictor& set_allowed(VW::v_array<action>& a);

  // set/add allowed but with per-actions costs specified
  predictor& add_allowed(action a, float cost);
  predictor& add_allowed(action* a, float* costs, size_t action_count);
  predictor& add_allowed(std::vector<std::pair<action, float> >& a);

  predictor& set_allowed(action a, float cost);
  predictor& set_allowed(action* a, float* costs, size_t action_count);
  predictor& set_allowed(std::vector<std::pair<action, float> >& a);

  // add a tag to condition on with a name, or set the conditioning
  // variables (i.e., erase previous ones)
  predictor& add_condition(ptag tag, char name);
  predictor& set_condition(ptag tag, char name);
  predictor& add_condition_range(
      ptag hi, ptag count, char name0);  // add (hi,name0), (hi-1,name0+1), ..., (h-count,name0+count)
  predictor& set_condition_range(
      ptag hi, ptag count, char name0);  // set (hi,name0), (hi-1,name0+1), ..., (h-count,name0+count)

  // set learner id
  predictor& set_learner_id(size_t id);

  // change the current tag
  predictor& set_tag(ptag tag);

  // make a prediction
  action predict();

private:
  bool is_ldf;
  ptag my_tag;
  VW::example* ec;
  size_t ec_cnt;
  std::vector<VW::example> allocated_examples;
  float weight;
  VW::v_array<action> oracle_actions;
  VW::v_array<ptag> condition_on_tags;
  VW::v_array<char> condition_on_names;
  VW::v_array<action> allowed_actions;
  VW::v_array<float> allowed_actions_cost;
  size_t learner_id;
  search& sch;

  // prevent the user from doing something stupid :) ... ugh needed to turn this off for python :(
  // predictor(const predictor&P);
  // predictor&operator=(const predictor&P);
};

// some helper functions you might find helpful
/*template<class T> void check_option(T& ret, VW::workspace&all, po::variables_map& vm, const char* opt_name, bool
default_to_cmdline, bool(*equal)(T,T), const char* mismatch_error_string, const char* required_error_string) { if
(vm.count(opt_name)) { ret = vm[opt_name].as<T>(); *all.args_n_opts.file_options << " --" << opt_name << " " << ret;
  }
  else if (strlen(required_error_string)>0)
  { std::cerr << required_error_string << std::endl;
    if (! vm.count("help"))
      THROW(required_error_string);
  }
  }*/

// void check_option(bool& ret, VW::workspace&all, po::variables_map& vm, const char* opt_name, bool default_to_cmdline,
// const char* mismatch_error_string);
}  // namespace Search

namespace VW
{
namespace reductions
{
std::shared_ptr<VW::LEARNER::learner> search_setup(VW::setup_base_i& stack_builder);
}
}  // namespace VW
