#include "base.h"
#include "type_reflection.h"
#include "type_constructors.h"

namespace typesys
{
template <typename WrapperT>
using is_known_field_wrapper_t = std::enable_if_t<std::is_same_v<field_kind, decltype(WrapperT::eftype())>>;

template <typename self_t, const type_descriptor* td>
struct tc_data_base
{
  template <typename T, T(self_t::*ptr)>
  using field_id = field_ptr_id<self_t,T,ptr>;

protected:
  

  // template <typename FieldT, 
  //           template<typename _> typename Wrapper = Prop, 
  //           typename = is_known_field_wrapper_t<Wrapper<FieldT>>
  //           >
  // struct field_builder
  // {
  //   using field_ptr_t = Wrapper<FieldT>(T::*);

  //   field_builder(std::string name, field_ptr_t ptr)
  //     : eftype{ Wrapper<FieldT>::eftype() }, name{name}
  //   {
  //   }

  //   private:
  //     erased_field_type eftype;
  //     std::string name;

  //     template<typename WrapperT>
  //     static erased_field_type init_field_type()
  //     {
  //       const field_kind kind = WrapperT::_field_kind;
  //       switch (kind)
  //       {
  //       case field_kind::scalar:
  //         return erased_field_type::scalar(type<FieldT>::erase());
  //       case field_kind::vector:
  //         return erased_field_type::vector(type<FieldT>::erase());
  //       case field_kind::map:
  //         return erased_field_type::map(type<typename WrapperT::_key_type>::erase(), type<FieldT>::erase());
  //       }
  //     }
  // };
};

/*
TC_DATA(my_data)
{
  prop((int) x)
  prop((std::string) y)

  vec((float) z)
  map((std::string, float) w)
}

TC_EXTENSION(my_data2) OF(my_data)
{
  prop((bool) flag)
}
*/
//*/

struct my_data
{
  using self_t = my_data;
  template <typename T, T(self_t::*ptr)>
  using field_id = field_ptr_id<self_t,T,ptr>;

  // prop_((int) x)
  //static field_id<Prop<int>,&self_t::x> t_x_id;;
  Prop<int> x;
  inline static auto x_id() { return field_id<Prop<int>,&self_t::x>(); }

  // prop_((std::string) y)
  Prop<std::string> y;
  inline static auto y_id() { return field_id<Prop<std::string>,&self_t::y>(); }
};

void test_reflect()
{
  using std::cout;
  using std::endl;

  cout << "Testing Reflection and AutoReflection" << endl;

  type_descriptor my_data_descriptor { "my_data", type<my_data>::erase() };

  erased_field_binder x_field_binder = my_data::x_id()().erase_binder();
  property_descriptor x_desc { "x", erased_field_type::scalar(type<int>::erase()), x_field_binder };
  my_data_descriptor.register_property(std::move(x_desc));

  erased_field_binder y_field_binder = my_data::y_id()().erase_binder();
  property_descriptor y_desc { "y", erased_field_type::scalar(type<std::string>::erase()), y_field_binder };
  my_data_descriptor.register_property(std::move(y_desc));

  //g_registry.types.emplace_back(std::move(my_data_descriptor));

  cout << "Testing Registered types:" << endl;
  // for (const auto& td : g_registry.types)
  // {
  //   cout << "  " << td.name << endl;
  // }
}

}  // namespace typesys