#include "test_v3.h"

#include "type_erase.h"
#include "reflection.h"
#include "type_reflection.h"
#include "type_constructors.h"

using typesys::Prop;
using typesys::UMap;

struct DataStruct
{
  Prop<int> a;

  DataStruct()
  {

  }
};

template <typename C, typename T, template<typename _> typename Wrapper = Prop>
struct PropertyBuilder
{
  struct property_desc
  {
    std::string _name;
    typesys::erased_type _type;
    typesys::erased_field_binder _binder;
  };

  using field_ptr_t = Wrapper<T>(C::*);

  property_desc _desc;

  PropertyBuilder()
  {
    _desc._type = type<T>::erase();
  }

  PropertyBuilder& for_type(typesys::type_descriptor& td)
  {

    return *this;
  }

  PropertyBuilder& with_name(const char* name)
  {
    _desc._name = name;
    return *this;
  }

  PropertyBuilder& with_field_ptr(field_ptr_t ptr)
  {
    _desc._binder = typesys::field_ptr<C, T>{ptr}.get_binder();
    return *this;
  }

  // implicit cast operator to Prop<T>
  operator Prop<T>() const
  {
    // how do we ensure we can get the property_desc from somewhere?

    return Prop<T>{};
  }
};

template <typename K>
struct MapHelper
{
  template <typename V>
  using Wrapper = UMap<K, V>;
};

template <typename C, typename K, typename V>
using MapPropertyBuilder = PropertyBuilder<C, UMap<K, V>, MapHelper<K>::template Wrapper>;

struct ReflectStruct
{
  using self_t = ReflectStruct;
  //template <typename T>
  //using FieldId = 

  // prop_( (int) a )
  Prop<int> a = PropertyBuilder<self_t, int>().with_name("a").with_field_ptr(&self_t::a);
  //static typesys::field_ptr_id<self_t, Prop<int>, &self_t::a> Field_a;
  //static constexpr char* Field_a_name = "a";


  Prop<std::string> b;


};

struct option_descriptor_tag
{};

extern std::string ESSENTIAL("");

template <typename T, typename PropT>
struct option_descriptor : option_descriptor_tag
{
private:
  std::string _name;
  std::string _help;
  std::vector<std::string> _aliases;
  typesys::field_ptr<T, PropT> _field;

  // todo: rest of option_builder stuff

private:
  

public:
  inline option_descriptor(typesys::field_ptr<T, PropT> field)
    : _field(field)
  {
  }

  inline option_descriptor(const option_descriptor& other)
    : _name(other._name)
    , _help(other._help)
    , _aliases(other._aliases)
    , _field(other._field)
  {
  }

  inline option_descriptor(option_descriptor&& other)
    : _name(std::move(other._name))
    , _help(std::move(other._help))
    , _aliases(std::move(other._aliases))
    , _field(std::move(other._field))
  {
  }

  inline option_descriptor& with_name(std::string name)
  {
    _name = name;
    return *this;
  }

  inline option_descriptor& with_help(std::string help)
  {
    _help = help;
    return *this;
  }

  inline bool is_essential() const
  {
    return _name == ESSENTIAL;
  }

  inline std::string name() const
  {
    return _name;
  }

  inline std::string help() const
  {
    return _help;
  }

  inline std::vector<std::string> aliases() const
  {
    return _aliases;
  }

  inline typesys::field_ptr<T, PropT> field() const
  {
    return _field;
  }
};

struct reduction_options
{

};

struct V3ReductionDescriptor
{
  struct hyper_param_t {
    int a;
  };

  struct predict_param_t {
    int b;
  };

  struct learn_param_t {
    int c;
  };

};

struct non_def_constructible
{
  int a;

  non_def_constructible(int a) : a(a)
  {
    std::cout << "constructed" << std::endl;
  }

  inline static non_def_constructible* default_init()
  {
    return new non_def_constructible(0);
  }

  inline static void default_destroy(non_def_constructible* p)
  {
    delete p;
  }
};

struct example_t {};
struct label_t {};

// implement ostream operator<< for Prop<T>
template <typename T>
std::ostream& operator<<(std::ostream& os, const Prop<T>& p)
{
  os << p.val;
  return os;
}

void t3_learn(DataStruct& data, example_t& ex, const label_t& l)
{
  std::cout << "learn with data.a = " << data.a << std::endl;
}

template <typename LearnerDataT, void(*typed_learn)(LearnerDataT&, example_t&, const label_t&)>
void erased_learn(typesys::activation& data, example_t& ex, const label_t& l)
{
  auto& d = data.get<LearnerDataT>();
  typed_learn(d, ex, l);
}

using erased_learn_f = void(*)(typesys::activation&, example_t&, const label_t&);

namespace test_v3
{
void test_property_binding()
{
  using namespace typesys;
  using std::cout;
  using std::endl;

  // what does the ideal syntax look like?
  auto AFieldPointer = field_ptr_id<DataStruct, Prop<int>, &DataStruct::a>()();
  auto ADescriptor = option_descriptor<DataStruct, Prop<int>>(AFieldPointer).with_help("a property").with_name("a");

  auto erased = type<DataStruct>::erase();
  activation a = erased.activator();

  ref a_ref(a.get<DataStruct>()); // todo: we really need a way to go from an erased type and activation to a ref
  erased_lvalue_ref elv { erased, a_ref }; // or do we?

  Prop<int>* p;
  if (AFieldPointer.try_bind(elv, p))
  {
    cout << "bound" << endl;
    p->val = 2;
  }

  // now if we were going to actually call into a learn method, we should be able to do something like this:
  erased_learn_f learn_f = &erased_learn<DataStruct, t3_learn>;
  
  // then the driver just needs to have
  example_t ex;
  label_t l;

  // to do
  learn_f(a, ex, l);

  // type-erase non-default constructible type
  // auto erased = type<non_def_constructible>::erase<&non_def_constructible::default_init, &non_def_constructible::default_destroy>();
  // activation a = erased.activator();

  //cout << "built descriptors" << endl;
  // okay, so a reduction's options is just a set of option_descriptors
}


}

// struct T1{};
// struct T2{};

namespace typesys { void test_reflect(); }

void test_v3_main()
{
  typesys::test_reflect();

  // std::type_index vti = typeid(void);
  // std::type_index ti1 = typeid(T1);
  // std::type_index ti2 = typeid(struct T2);

  // std::cout << vti.name() << std::endl;
  // std::cout << ti1.name() << " == " << ti2.name() << ": " << (ti1 == ti2) << std::endl;
  //test_v3::test_property_binding();
}