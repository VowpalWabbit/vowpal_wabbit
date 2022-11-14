#include "prelude.h"
#include "autofb.h"

#include "typeerase.h"

#include "reductions.h"
#include "kernel_svm.h"

#include "v2/test.h"

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

extension(derived) of (base)
{
  _(double, l1);
  _(double, l2);
};

// a type erased object needs to have the following capabilities
//  - sized storage that can be 

int main(int argc, char** argv)
{
  //test();
  //test_autofb();
  //kernel_svm_test();
  do_test();
}

template <typename builtin>
void print_type()
{
  std::cout << typeid(builtin).name() << std::endl;
}

void test()
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
  erased_effect_table dt;

  // register a lambda to print the type name for each of the types
  dt.add<int32_t, print_type<int32_t>>()
    .add<uint32_t, print_type<uint32_t>>()
    .add<float, print_type<float>>()
    .add<bool, print_type<bool>>()
    .add<unsigned char, print_type<unsigned char>>()
    .add<double, print_type<double>>();
  
  {

  base b; // unfortunately, due to static lifetime initialization, we must construct a prototype of the object
          // before we can reflect over its properties
          // the expectation is that all of these objects need to be trivially constructable
  derived d;
  
  } // note, however, that we do NOT need to have these objects still be live to make use of the reflection information

  std::for_each(types.types_begin(), types.types_end(), [&dt](const typesys::type_info& type_info)
  {
    cout << "Type: " << type_info.name.name << "_V" << type_info.name.version;
    if (type_info.has_base())
    {
      auto& base_type_info = universe::instance().get_type(type_info.base());
      cout << " : " << base_type_info.name.name << "_V" << base_type_info.name.version;
    }
    cout << endl;

    //auto properties = type_info.properties;
    std::for_each(type_info.props_begin(), type_info.props_end(), [&dt](const typesys::property_info& property_info)
    {
      cout << "  Property: " << property_info.name << ": ";
      dt.dispatch(property_info.etype);
      cout << endl;
    });
  });
}