#include <iostream>

#include "autofb.h"

using namespace typesys;
using std::cout;
using std::endl;

struct printing_property_visitor
{
  template <typename T>
  void visit_property(const char* name) // todo: property index?
  {
    cout << name << ": " << typeid(T).name() << endl;
  }

  template <typename T>
  void visit_property_value(const char* name, const T& value)
  {
    cout << name << " = " << value << endl;
  }
};

struct printing_type_visitor
{
  void begin_type(const versioned_name& type)
  {
    cout << "BeginType(" << type.name << "_V" << type.version << ")" << endl;
  }

  using property_visitor = printing_property_visitor;

  void end_type(const versioned_name& type)
  {
    cout << "EndType(" << type.name << "_V" << type.version << ")" << endl;
  }
};

template <typename T, int V>
class reflector
{
private:
  static inline typesys::type_index type_index()
  {
    return universe::instance().get_type_index(versioned_name::make_name<T, V>());
  }

public:
  template <typename visitor_t>
  static void visit_type()
  {
    visitor_t::begin_type(vname);

    auto type_info = universe::instance().get_type(type_index());
    for (auto& property : type_info.properties)
    {
      // TODO: How to unerase the type here?
      //visitor_t::property_visitor::visit_property<>(property.name);
    }
  }

  template <typename visitor_t>
  static void visit_value(const T& value)
  {
    
  }
};