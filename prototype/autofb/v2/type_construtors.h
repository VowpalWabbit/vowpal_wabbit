#pragma once

#include "base.h"

#include "type_registry.h"
#include "staticinit.h"
#include "type_erase.h"

#include "beacon.h"

#define DATA_KIND struct

namespace type_system
{
  using scalar_reflector_f = std::function<erased_lvalue()>;
  using vector_reflector_f = std::function<erased_vector()>;

  struct reflector_map
  {
    using scalar_map_t = std::unordered_map<std::string, scalar_reflector_f>;
    using vector_map_t = std::unordered_map<std::string, vector_reflector_f>;

    scalar_map_t scalar_map;
    vector_map_t vector_map;
  };

  class reflector;

  DATA_KIND reflectable
  {
  private:
    reflector_map reflection_map;

  protected:
    beacon<reflector_map> reflection_beacon = beacon<reflector_map>(&reflection_map);
    reflectable& as_reflectable() { return *this; };
    friend class reflector;
  };

  class reflector
  {
  // public:
  //   static const type_descriptor& descriptor() { return type_registry::instance().lookup_type(registration); }

  public:
    reflector(reflectable& target) : _target(target), _map(target.reflection_map)
    {
    }

    erased_lvalue reflect_scalar(name_t name)
    {
      auto it = _map.scalar_map.find(name);
      if (it == _map.scalar_map.end())
        // todo: figure out error handling
        throw std::runtime_error("reflector::reflect_scalar: name not found");

      return it->second();
    }

    erased_vector reflect_vector(name_t name)
    {
      auto it = _map.vector_map.find(name);
      if (it == _map.vector_map.end())
        // todo: figure out error handling
        throw std::runtime_error("reflector::reflect_scalar: name not found");

      return it->second();
    }
    
  private:
    reflectable& _target;
    const reflector_map& _map;
  };

  template <typename T, typename container_t>
  class scalar_property
  {
  public:
    scalar_property(beacon<reflector_map>& reflector, const property_descriptor& pd) : 
      _value{}, etype(pd.etype), name(pd.name), cobeacon{reflector.unchase(this)}
    {
      register_reflector();
      //reflector.scalar_map[pd.name] = [this]() { return this->erased(); };
    }

    scalar_property(const scalar_property& other)
    : _value(other._value), etype(other.etype), name(other.name), cobeacon{other.cobeacon}
    {
      register_reflector();
    }

    scalar_property(scalar_property&& other)
    : _value(std::move(other._value)), etype(other.etype), name(other.name), cobeacon{other.cobeacon}
    {
      register_reflector();
    }

    // TODO: Rest of Rule of N

    void operator=(const T& value) { _value = value; }

    scalar_property& operator=(T&& value)
    {
      _value = std::move(value);
      return *this;
    }

    scalar_property& operator=(const scalar_property& other)
    {
      _value = other._value;
      etype = other.etype;
      cobeacon = other.cobeacon;
      return *this;
    }

    inline erased_lvalue erased() { return erased_lvalue{etype, ref(_value)}; }

    operator T() const { return _value; }
    operator T&() { return _value; }

    T& operator()() { return _value; }

  private:
    using selftype = scalar_property<T, container_t>;

    void register_reflector()
    {
      cobeacon.chase(this).get().scalar_map[name] = [this]() { return this->erased(); };
    }

    T _value;
    name_t name;
    erased_type etype;
    beacon<reflector_map>::cobeacon<selftype> cobeacon;
    //erased_lvalue _erased;
  };

  template <typename T, typename container_t>
  std::ostream& operator<<(std::ostream& os, const scalar_property<T, container_t>& p)
  {
    os << T(p);
    return os;
  }

  template <typename T, typename container_t>
  class vector_property
  {
  public:
    using iter = typename std::vector<T>::const_iterator;

    vector_property(const beacon<reflector_map>& reflector, const property_descriptor& pd) : 
      _value{}, etype(pd.etype), name(pd.name), cobeacon(reflector.unchase(this))
    {
      register_reflector();
    }

    vector_property(const vector_property& other)
    : _value(other._value), etype(other.etype), name(other.name), cobeacon(other.cobeacon)
    {
      register_reflector();
    }

    vector_property(vector_property&& other)
    : _value(std::move(other._value)), etype(other.etype), name(other.name), cobeacon(other.cobeacon)
    {
      register_reflector();
    }

    static erased_vector get_erased(void* target)
    {
      return reinterpret_cast<vector_property<T, container_t>*>(target)->erased();
    }

    T& operator[](std::size_t index) { return _value[index]; }
    const T& operator[](std::size_t index) const { return _value[index]; }

    void push_back(const T& value) { _value.push_back(value); }
    void push_back(T&& value) 
    {
      std::cout << "push_back(T&&)";
      if (std::is_same<T, float>::value)
      {
        std::cout << " [is float: " << *(float*)&value << "]";
      }

      _value.push_back(std::move(value));

      std::cout << std::endl << "  size: " << _value.size() << std::endl;
    }

    void pop_back() { _value.pop_back(); }

    void assign(const void* begin, const void* end) { _value.assign((T*)begin, (T*)end); }

    const T* cdata() const { return _value.data();}
    iter begin() const { return _value.begin(); }
    iter end() const { return _value.end(); }

    std::size_t size() const { return _value.size(); }
    
    void reserve(std::size_t size) { _value.reserve(size); }
    void resize(std::size_t size) { _value.resize(size); }

    inline erased_vector erased() 
    { 
      return vtype<T>::erase(_value); 
    }

    vector_property& operator=(const vector_property& other)
    {
      _value = other._value;
      etype = other.etype;
      cobeacon = other.cobeacon;
      return *this;
    }

    std::vector<T>& operator()() { return _value; }

  private:
    using selftype = vector_property<T, container_t>;

    void register_reflector()
    {
      cobeacon.chase(this).get().vector_map[name] = [this]() { return this->erased(); };
    }

    std::vector<T> _value;
    name_t name;
    erased_type etype;
    beacon<reflector_map>::cobeacon<selftype> cobeacon;
    
  };

  using type_registration_evidence = staticinit::evidence<type_number>;
  using type_activation_evidence = staticinit::evidence<erased_type>;

  template <typename actual_t,
    const type_registration_evidence& registration,
    const type_activation_evidence& activation>
  DATA_KIND data : public reflectable
  {
    public:
      static type_number ts_index() { return registration; }
      static type_descriptor& descriptor() { return type_registry::instance().lookup_type(ts_index()); }
      static erased_type etype() { return activation; }

    public:
      static property_descriptor& register_property(const char* name, erased_type etype, bool is_vector)
      {
        return descriptor().define_property(name, etype, is_vector);
      }

      static property_descriptor& register_property(const char* name, erased_type etype, bool is_vector, std::initializer_list<attribute> attributes)
      {
        return descriptor().define_property(name, etype, is_vector, attributes);
      }

    public:
      data()
      {
        register_this_reflection();
      }

      data(const data& other)
      {
        register_this_reflection();
      }

      data& operator=(const data& other)
      {
        return *this;
      }

    protected:
      template <typename T>
      using property_ = scalar_property<T, actual_t>;

      template <typename T>
      using vector_ = vector_property<T, actual_t>;

    private:
      void register_this_reflection()
      {
        this->reflection_beacon.get().scalar_map["this"] = [this]() 
        { 
          return erased_lvalue{etype(), ref(*this)}; 
        };
      }

    protected:
      using reflectable::reflection_beacon;

    public:
      using reflectable::as_reflectable;
  };

  template <typename actual_t,
    const type_registration_evidence& registration,
    const type_activation_evidence& activation,
    typename base_t>
  DATA_KIND extension : private data<actual_t, registration, activation>, public base_t
  {
    using data<actual_t, registration, activation>::descriptor;
    using data<actual_t, registration, activation>::etype;
    
    using data<actual_t, registration, activation>::property_;
    using data<actual_t, registration, activation>::vector_;
    using data<actual_t, registration, activation>::register_property;

    protected:
      using base_t::reflection_beacon;

    extension()
    {
    }
  };

  template <name_t name, typename T>
  type_number register_data_type()
  {
    return type_registry::instance().register_type(name, type<T>::erase()).typeindex;
  }

  template <const type_registration_evidence& registration, const type_registration_evidence& base_registration>
  type_number extend_data_type()
  {
    registration._value.set_base(base_registration._value.typeindex);

    return registration._value.typeindex;
  }

  template <typename T, const type_registration_evidence& registration>
  erased_type attach_activator()
  {
    erased_type etype = type<T>::erase();
    type_descriptor& descriptor = type_registry::instance().lookup_type(registration);
    
    descriptor.set_activator(etype.activator);
    return etype;
  }
}



#define __TC_DETAILS(type_name) __ ## type_name ## _details
#define __TC_REGISTER_TYPE(type_name) DATA_KIND type_name; \
  namespace __TC_DETAILS(type_name) { \
    using namespace type_system; \
    extern const char name[] = #type_name; \
    using registration_witness = staticinit::witness<type_number, &register_data_type<name, type_name>>; \
    static const type_system::type_registration_evidence registration = registration_witness::testify(); \
    using activation_witness = staticinit::witness<erased_type, &attach_activator<type_name, registration>>; \
    static const type_system::type_activation_evidence activation = activation_witness::testify(); \
  }

#define __TC_EXTEND_TYPE(type_name, base_type_name) \
  namespace __TC_DETAILS(type_name) { \
    using _T = type_name; \
    using extension_witness = staticinit::witness<type_number&, &extend_data_type<registration, __TC_DETAILS(base_type_name)::registration>>; \
    static const type_system::type_registration_evidence extension = extension_witness::testify(); \
  }

#define TC_DATA(type_name) __TC_REGISTER_TYPE(type_name) \
  DATA_KIND type_name : private type_system::data<type_name, __TC_DETAILS(type_name)::registration, __TC_DETAILS(type_name)::activation>

#define DERIVE_REFLECT(type_name) public: using type_system::data<type_name, __TC_DETAILS(type_name)::registration, __TC_DETAILS(type_name)::activation>::as_reflectable;

#define __TC_PROP_REGISTRATOR(name) __register_ ## name
#define __TC_PROP_WITNESS(name) __witness ## name
#define __TC_PROP_WITNESS_TYPE(name) staticinit::witness<property_descriptor&, &__TC_PROP_REGISTRATOR(name)>

#define __TC_REGISTER_PROPERTY(_type, name, is_vector) private: \
  inline static property_descriptor& __TC_PROP_REGISTRATOR(name)() { return register_property(#name, type<_type>::erase(), is_vector); }; \
  inline static __TC_PROP_WITNESS_TYPE(name) __TC_PROP_WITNESS(name)() { static __TC_PROP_WITNESS_TYPE(name) w; return w; };

#define _(_type, name) __TC_REGISTER_PROPERTY(_type, name, false) \
  public: property_<_type> name = { reflection_beacon, __TC_PROP_WITNESS(name)().get_evidence() }

#define v_(_type, name) __TC_REGISTER_PROPERTY(_type, name, true) \
  public: vector_<_type> name = { reflection_beacon, __TC_PROP_WITNESS(name)().get_evidence() }