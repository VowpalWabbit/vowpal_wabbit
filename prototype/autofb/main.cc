#include "prelude.h"
#include "autofb.h"

#include <algorithm>
#include <functional>

#include "typeerase.h"

using namespace typesys;

//register_type(base, 1);
//using base_tr = detail::registration<"base", 1>;

data(base)
{
//private: static property_witness<unsigned char>& b_propinfo() { static property_witness<unsigned char> pi{"b"}; return pi; } 
//public: __<unsigned char> name = __<unsigned char>("b", b_propinfo().info);
  _(unsigned char, b);
  //_(float, eta);
};

// extension(derived) of (base)
// {
//   _(double, l1);
//   _(double, l2);
// };





// a type erased object needs to have the following capabilities
//  - sized storage that can be 

#define REDUCTION_SIGNATURE typename init_params_t, typename predict_params_t, typename learn_params_t
#define REDUCTION_PARAMS init_params_t, predict_params_t, learn_params_t

template <REDUCTION_SIGNATURE>
struct reduction_data 
{
  init_params_t init_params;
  predict_params_t predict_params;
  learn_params_t learn_params;
};

struct empty_t {};

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

bool enable_reduction (options_t options, empty_t const& init_params) { return true; }
learner_t init_reduction (stack_builder_t stack_builder, reduction_data<empty_t, empty_t, empty_t> const& data) { return nullptr; }

template <typename builtin>
void print_type()
{
  std::cout << typeid(builtin).name() << std::endl;
}

int main(int argc, char** argv)
{
  using empty_reduction_factory = reduction_factory<empty_t, empty_t, empty_t, init_reduction, enable_reduction>;
  auto legacy_setup_fn = &empty_reduction_factory::setup;

  empty_reduction_factory actual_factory;
  actual_factory.setup_erased();

  //b.int_prop = 1;

  using std::cout;
  using std::endl;

  universe& types = universe::instance();

                                               // \/ this ONLY works because of the IMPL detail that it is a vector
  cout << "Types in the universe: (Count = " << (types.types_end() - types.types_begin()) << ")" << endl;

  // dispatch table for BUILTIN property types (for now int32 types, float type and bool type)
  erased_dispatch_table dt;

  // register a lambda to print the type name for each of the types
  dt.add<int32_t, print_type<int32_t>>()
    .add<uint32_t, print_type<uint32_t>>()
    .add<float, print_type<float>>()
    .add<bool, print_type<bool>>()
    .add<unsigned char, print_type<unsigned char>>();
  

  base b; // unfortunately, due to static lifetime initialization, we must construct a prototype of the object
          // before we can reflect over its properties
          // the expectation is that all of these objects need to be trivially constructable
  std::for_each(types.types_begin(), types.types_end(), [&dt](const typesys::type_info& type_info)
  {
    cout << "Type: " << type_info.name.name << "_V" << type_info.name.version << endl;
    //auto properties = type_info.properties;
    std::for_each(type_info.props_begin(), type_info.props_end(), [&dt](const typesys::property_info& property_info)
    {
      cout << "  Property: " << property_info.name << ": ";
      dt.dispatch(property_info.etype);
      cout << endl;
    });
  });
}