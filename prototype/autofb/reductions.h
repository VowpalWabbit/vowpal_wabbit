#pragma once


struct empty_t {};

#define REDUCTION_SIGNATURE typename init_params_t, typename predict_params_t, typename learn_params_t
#define REDUCTION_PARAMS init_params_t, predict_params_t, learn_params_t

template <REDUCTION_SIGNATURE>
struct reduction_data 
{
  init_params_t init_params;
  predict_params_t predict_params;
  learn_params_t learn_params;
};

using stack_builder_t = void*;
using options_t = void*;
using learner_t = void*;

inline options_t get_options_from_builder(stack_builder_t stack_builder) { return nullptr; }

template <REDUCTION_SIGNATURE>
struct reduction_descriptor
{
  using data_t = reduction_data<init_params_t, predict_params_t, learn_params_t>;

  using parse_options_fn = bool(*)(options_t options, init_params_t const& init_params);
  using init_reduction_fn = learner_t(*)(stack_builder_t stack_builder, data_t const& data);
};

template <REDUCTION_SIGNATURE>
using _enable_reduction_fn = bool(*)(options_t options, init_params_t const& init_params);
#define enable_reduction_fn _enable_reduction_fn<REDUCTION_PARAMS>

template <REDUCTION_SIGNATURE>
using _init_reduction_fn = learner_t(*)(stack_builder_t, reduction_data<init_params_t, predict_params_t, learn_params_t> const&);
#define init_reduction_fn _init_reduction_fn<REDUCTION_PARAMS>



template <REDUCTION_SIGNATURE, init_reduction_fn init_reduction, enable_reduction_fn enable_reduction>
struct reduction_factory
{
  using data_t = reduction_data<init_params_t, predict_params_t, learn_params_t>;

  struct init_closure
  {
    data_t data;

    learner_t invoke(stack_builder_t stack_builder) { return init_reduction(stack_builder, data); };
  };

  void setup_erased()
  {
    stack_builder_t stack_builder;

    // the goal here is to do the following:
    //   parse the options into a closure for the initialization
    init_closure closure;

    //std::function fn = std::bind(&init_closure::invoke, closure, stack_builder);
  }

  static learner_t setup(void* stack_builder)
  {
    options_t options = get_options_from_builder(stack_builder);

    init_params_t init_params;
    if (!enable_reduction(options, init_params))
      return nullptr;

    data_t data{init_params};
    return init_reduction(stack_builder, data);
  }
};

inline bool enable_reduction (options_t options, empty_t const& init_params) { return true; }
inline learner_t init_reduction (stack_builder_t stack_builder, reduction_data<empty_t, empty_t, empty_t> const& data) { return nullptr; }
