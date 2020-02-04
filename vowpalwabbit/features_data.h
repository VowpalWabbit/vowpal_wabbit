#pragma once

#include <array>
#include <memory>
#include <type_traits>

/*
 * class features_data
 * Description:
 *   This data structure manages access and lifetime of the features data used in various VW reductions.
 *     New types must be created with the templatized create() function call, attempting to access
 *     uninitialized data will return a nullptr.
 *
 *   All contained data types MUST instantiate a `clear()` function that readies the object for reuse.
 *
 *   Calling create() on an existing element will clear and overwrite the existing data
 *
 * Member functions:
 *   T* create(constructor_parameters);
 *   T* get();
 *   void clear();
 *
 * Data structure usage:
 *   Add new data structure to the available types:
 *     Add a new line to the TYPE_MAPPINGS macro
 *
 *   Create an instance of a data structure:
 *     features_data fd;
 *     fd.create<data_type>(constructor_parameters);
 *
 *   Accessing an instance of existing data:
 *     auto data = fd.get<data_type>();
 */

/////////////////////////////////////////////
// sample placeholder structs. For demonstrative purposes only
struct basic_features{
basic_features() : x(0) {}
  int x;
  void clear();
};
struct ccb_features
{
  int x;
  float y;
  double z;
  void clear();
};
/////////////////////////////////////////////

/*
 * Escape the last line and add new data structure->enum mappings to the end of this list
 */
#define TYPE_MAPPINGS        \
  MAP(basic_features, basic) \
  MAP(ccb_features, ccb)
// Escape the last line and add new mappings to the end of this list


////////////////////////////////////////////////////////////////////
// ----------------- DO NOT MODIFY BELOW THIS LINE -----------------
////////////////////////////////////////////////////////////////////

// Construct the enum class based on the TYPE_MAPPINGS. Last element is always END
#define MAP(TYPE, ENUM) ENUM,
enum class  features_type_t
{
  TYPE_MAPPINGS
  END
};
#undef MAP

// Decay an enum class value to its underlying type
template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// Used to check the existence of a method called `clear()`
template <typename T>
struct has_clear {
private:
  typedef char YesType[1];
  typedef char NoType[2];

  template <typename C> static YesType& test( decltype(&C::clear) );
  template <typename C> static NoType& test(...);

public:
  enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

// type_traits for the mapped types. Can convert a type to its enumerated value.
// These are only enabled if the type contains a member function called `clear()`
// This struct is instrumental in all data access, so the clear() function will
// always be required
template <typename T>
struct type_traits;
#define MAP(TYPE, ENUM)                                                 \
template <>                                                             \
struct type_traits<TYPE> {                                              \
  static constexpr features_type_t enum_value = features_type_t::ENUM;  \
  static constexpr size_t index = to_underlying(enum_value);            \
};

TYPE_MAPPINGS
#undef MAP

/*
 * features_data wraps an array of shared_ptr<void>. The shared pointers are constructed
 * as their appropriate types and stored as void pointers. Upon retrieval, they are cast back to
 * their original type. This is safe because access is managed entirely through the type_traits
 * mapping constructed via macros at compile time
 */
class features_data {
private:
std::array<std::shared_ptr<void>, to_underlying(features_type_t::END)> _data;

public:
 template <typename T, class... Args>
 typename std::enable_if<has_clear<T>::value, T>::type*
  create(Args&&... args) {
   T* ret = get<T>();
   if(ret) {
     ret->clear();
     new (ret) T(std::forward<Args>(args)...);
   }
   else
   {
     ret = (T*)(_data[type_traits<T>::index] = std::make_shared<T>(std::forward<Args>(args)...)).get();
   }
   return ret;
 }
 template <typename T>
 T* get() { return (T*)(_data[type_traits<T>::index].get()); }

 // call clear() on all instantiated types
 void clear(){
#define MAP(TYPE, ENUM)       \
    {                         \
      auto ptr = get<TYPE>(); \
      if (ptr) ptr->clear();  \
    }
   TYPE_MAPPINGS
#undef MAP
 }
};

#undef TYPE_MAPPINGS
