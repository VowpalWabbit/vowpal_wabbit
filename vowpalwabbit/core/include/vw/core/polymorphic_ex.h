#include "vw/core/multi_ex.h"

#include <type_traits>
#include <typeinfo>

namespace VW
{
// VW::is_nonqualified_example_type<ExampleT> will check if a given type is exactly VW::example or VW::multi_ex
template <class ExampleT>
struct is_nonqualified_example_type
    : std::integral_constant<bool,
          std::is_same<VW::example, ExampleT>::value || std::is_same<VW::multi_ex, ExampleT>::value>
{
};

// VW::is_example_type<ExampleT> will check if a given type is (maybe const) VW::example or VW::multi_ex
template <class ExampleT>
struct is_example_type
    : std::integral_constant<bool, VW::is_nonqualified_example_type<typename std::remove_cv<ExampleT>::type>::value>
{
};

// VW::is_multiline_type<ExampleT> will check if a given type is (maybe const) VW::multi_ex
template <class ExampleT>
struct is_multiline_type
    : std::integral_constant<bool, std::is_same<VW::multi_ex, typename std::remove_cv<ExampleT>::type>::value>
{
};

// Polymorphic wrapper around VW::example and VW::multi_ex
//
// The polymorphic_ex class is a pointer/reference wrapper that hides the
// underlying VW example type. It can be implicitly created from, and converted
// back to, any pointer or reference to a VW example type. In debug builds,
// polymorphic_ex will perform run-time type checking to ensure that its
// "input" and "output" types are the same, and also that the const-ness of its
// original type is not violated.
class polymorphic_ex
{
public:
  // Implicit conversion from both const and mutable pointer to polymorphic_ex
  template <class ExampleT, typename std::enable_if<VW::is_example_type<ExampleT>::value, bool>::type = true>
  polymorphic_ex(ExampleT* ex)
      : _ptr(ex)
      , _is_multiline(VW::is_multiline_type<ExampleT>::value)
#ifndef NDEBUG
      , _type(typeid(typename std::remove_cv<ExampleT>::type))
      , _is_const(std::is_const<ExampleT>::value)
#endif
  {
  }

  // Implicit conversion from both const and mutable reference to polymorphic_ex
  template <class ExampleT, typename std::enable_if<VW::is_example_type<ExampleT>::value, bool>::type = true>
  polymorphic_ex(ExampleT& ex)
      : _ptr(&ex)
      , _is_multiline(VW::is_multiline_type<ExampleT>::value)
#ifndef NDEBUG
      , _type(typeid(typename std::remove_cv<ExampleT>::type))
      , _is_const(std::is_const<ExampleT>::value)
#endif
  {
  }

  // Implicit conversion from polymorphic_ex to const pointer
  template <class ExampleT,
      typename std::enable_if<VW::is_nonqualified_example_type<ExampleT>::value, bool>::type = true>
  operator const ExampleT*() const
  {
#ifndef NDEBUG
    assert(_type == typeid(ExampleT));
#endif
    return static_cast<const ExampleT*>(_ptr);
  }

  // Implicit conversion from polymorphic_ex to const reference
  template <class ExampleT,
      typename std::enable_if<VW::is_nonqualified_example_type<ExampleT>::value, bool>::type = true>
  operator const ExampleT&() const
  {
#ifndef NDEBUG
    assert(_type == typeid(ExampleT));
#endif
    return *static_cast<const ExampleT*>(_ptr);
  }

  // Implicit conversion from polymorphic_ex to mutable pointer
  template <class ExampleT,
      typename std::enable_if<VW::is_nonqualified_example_type<ExampleT>::value, bool>::type = true>
  operator ExampleT*()
  {
#ifndef NDEBUG
    assert(_type == typeid(ExampleT));
    assert(!_is_const);
#endif
    return static_cast<ExampleT*>(const_cast<void*>(_ptr));
  }

  // Implicit conversion from polymorphic_ex to mutable reference
  template <class ExampleT,
      typename std::enable_if<VW::is_nonqualified_example_type<ExampleT>::value, bool>::type = true>
  operator ExampleT&()
  {
#ifndef NDEBUG
    assert(_type == typeid(ExampleT));
    assert(!_is_const);
#endif
    return *static_cast<ExampleT*>(const_cast<void*>(_ptr));
  }

  // Check if the polymorphic_ex was constructed with a multiline type
  bool is_multiline() const { return _is_multiline; }

private:
  const void* _ptr;
  const bool _is_multiline;
#ifndef NDEBUG
  const std::type_info& _type;
  const bool _is_const;
#endif
};

}  // namespace VW
