#include "prelude.h"

#pragma once

#include "staticinit.h"
#include "typeerase.h"
#include "type_activation.h"

#define DATA_KIND struct


struct erased_type;

namespace typesys
{
  using name_t = const char*;
  using version_t = unsigned int;

  struct versioned_name
  {
    const name_t name;
    const version_t version;

    struct hasher
    {
      std::size_t operator()(const versioned_name& k) const
      {
        using std::size_t;
        using std::hash;
        using std::string;

        return ((hash<string>()(k.name) ^ (hash<int>()(k.version) << 1)) >> 1);
      }
    };

    struct equal_to
    {
      bool operator()(const versioned_name& lhs, const versioned_name& rhs) const
      {
        return lhs.version == rhs.version && !strcmp(lhs.name, rhs.name);
      }
    };

    template <typename T, version_t V>
    inline static versioned_name make_name()
    {
      return { typeid(T).name(), V };
    }
  };

  using type_index = std::uint32_t;

  

  template <name_t N, version_t V>
  struct type_identity
  {
    static constexpr name_t name = N;
    static constexpr version_t version = V;

    static constexpr versioned_name versioned_name()
    {
      return { N, V };
    }

    inline static type_index register_type()
    {
      type_index index = universe::instance().register_type(versioned_name());
      return index;
    }
  };

  using property_index_t = size_t;

  struct property_info
  {
    const char* name;
    const erased_type etype;

    property_info(const char* name, erased_type etype) : name(name), etype(etype) {};
    property_info(property_info& other) = default;
  };

  template <typename T>
  void basic_activator() 
  { 
    static_assert(std::is_trivially_constructible<T>::value, "basic_activator only supports trivially constructible types"); 
    //static_assert(std::is_base_of<>); TODO: Add a "data" base tag, and "extension" base tag
    T activation_witness {};
  };

  struct type_info
  {
    const versioned_name name;
    const type_index index;
    
    type_info(versioned_name name, type_index index) : name{name}, index{index}, base_index{index} {}
    type_info(type_info& other) = default;

    inline void add_property(property_info pi) { properties.push_back(property_info{pi}); }
    inline void set_base(type_index base_index) { this->base_index = base_index; }
    inline void set_activator(activator_f activator) { this->activator = activator; }

    inline bool has_base() const { return base_index != index; }
    inline type_index base() const { return base_index; }

    inline bool has_activator() const { return activator != nullptr; }
    inline activation activate() const { return activator(); }
    
    using props_iter = std::vector<property_info>::const_iterator;

    inline props_iter props_begin() const { return properties.cbegin(); };
    inline props_iter props_end() const { return properties.cend(); };

  private:
    std::vector<property_info> properties;
    type_index base_index;
    activator_f activator;
  };

// todo: rearrange detail types
class universe
{
  public:
    static universe& instance();

    using type_map_t = std::unordered_map<versioned_name, type_index, 
                       versioned_name::hasher, 
                       versioned_name::equal_to>;
    using type_map_iter = type_map_t::const_iterator;

    using const_types_iter = std::vector<type_info>::const_iterator;

  public:
    inline type_map_iter find_type(const versioned_name& name) const
    {
      return typemap.find(name);
    }

    inline type_index register_type(const versioned_name& name)
    {
      return register_type_internal(name);
    }

    type_info& get_type(const versioned_name& name)
    {
      // find the type_id in typemap, then chain to get_type(type_id)
      // todo: what to do when not found?
      return get_type(name.name, name.version);
    }

    type_info& get_type(name_t name, version_t version)
    {
      // find the type_id in typemap, then chain to get_type(type_id)
      // todo: what to do when not found?
      return get_type(typemap[{name, version}]);
    }

    type_info& get_type(type_index id)
    {
      // todo: check range
      return typeinfos[id];
    }

    const_types_iter types_begin() const { return typeinfos.begin(); }
    const_types_iter types_end() const { return typeinfos.end(); }

    type_map_iter type_map_end() const { return typemap.end(); }

  private:
    universe();

    // template <typename T, typename = std::enable_if<!std::is_literal_type<T>::value>::type>
    // inline type_index register_builtin() { static_assert("only literal types are allowed"); }

    // template <typename T, typename = std::enable_if<std::is_literal_type<T>::value>::type>
    // inline type_index register_builtin() 
    // {
    //   register_type_internal({typeid(T).name(), 0}, TK_BUILTIN); 
    // }

    inline type_index register_type_internal(const versioned_name& name)
    {
      // todo: bounds check
      type_index id = static_cast<type_index>(typeinfos.size());

      typeinfos.push_back(type_info{name, id});
      typemap[name] = id;

      return id;
    }

    std::vector<type_info> typeinfos;

    // TODO: make the map lookup actually work by 
    type_map_t typemap;
};

namespace detail
{
  // template <typename concrete_t, typename base_t, int = base_t::version + 1>
  // struct extension_identity : type_identity<concrete_t, base_t::version + 1>
  // {
  //   using base_type_identity = type_identity<base_t, base_t::version>;
  // };

  template <typename T, typename container_t>
  class property
  {
    public:
      property(const char* name, const property_info&) : _value{}
      {
        //TODO: register property location with reflector
      }

      // TODO: Rest of Rule of N

      void operator=(const T& value) { _value = value; }
      property& operator=(T&& value) 
      {
        _value = std::move(value);
        return *this;
      }

      operator T() const { return _value; }
      operator T&() { return _value; }

    private:
      T _value;
  };

  using register_f = type_index(*)();

  struct registration_witness
  {
  private:
    registration_witness(versioned_name name, type_index index)
      : _type_index(index), 
        _name(name)
    {
    }

    template <name_t, version_t>
    friend class registration;

  public:
    const type_index _type_index;
    const versioned_name _name;
  };

  template <name_t N, version_t V>
  class registration
  {
  private:
    using tid = type_identity<N, V>;

    inline static type_index find_or_register()
    {
      auto iter = universe::instance().find_type(tid::versioned_name());
      if (iter == universe::instance().type_map_end())
      {
        return tid::register_type();
      }
      else
      {
        return iter->second;
      }
    }

    inline registration() : _witness(tid::versioned_name(), find_or_register()) {}

  public:
    static registration& instance()
    {
      static registration instance;
      return instance;
    }

    inline static const registration_witness witness()
    {
      return instance()._witness;
    }

    inline type_index index() const { return _registration._type_index; }
    inline type_info const& type() const { return universe::instance().get_type(_type_index); }

  private:
    registration_witness _witness;
  };

  using activation_witness = staticinit::evidence<erased_type>;

  template <typename concrete_t, const registration_witness& rw, const activation_witness& aw>
  DATA_KIND data
  {
    public:
      //static const int version = rw._name.version;
      static const erased_type& get_erased_type() { return aw._value; }
      static const typesys::type_info& get_tstype() { return universe::instance().get_type(rw._type_index); }

    public:
      data()
      {
        //std::cout << "intializing " << rw._name.name << std::endl;
      }

      ~data()
      {}

    public:
      template <typename T>
      using __ = property<T, concrete_t>;

      template <typename T>
      struct property_witness
      {
        property_info info;

        property_witness(const char* name) : info{name, type<T>::erase()} 
        {
          //std::cout << "in prop witness" << std::endl;
          universe::instance().get_type(rw._type_index).add_property(info);

          // here is where we can register the property with the reflection
          // infrastructure
          
        }
      };
  };

  template <typename concrete_t, const registration_witness& rw, const activation_witness& aw, typename base_t, const registration_witness& base_rw, const activation_witness& base_aw>
  DATA_KIND extension : private data<concrete_t, rw, aw>, public base_t
  {
    //using data<concrete_t, rw>::version;
    using data<concrete_t, rw, aw>::__;
    using data<concrete_t, rw, aw>::property_witness;

    extension()
    {
      // TODO: staticize this
      type_info& ti = universe::instance().get_type(rw._type_index);
      ti.set_base(base_rw._type_index);
    }
  };

} // namespace detail
} // namespace typesys

#define _(type, name) private: static property_witness< ## type ## >& name ## _propwitness() { static property_witness<type> pi{#name}; return pi; } \
  public: __<type> name = __<type>(#name, name ## _propwitness().info);

// TODO: Versions
#define details(type_name) __ ## type_name ## _details

template <typename T, const typesys::detail::registration_witness& rw>
erased_type attach_activator()
{
  //std::cout << "attaching activator for " << rw._name.name << std::endl;
  erased_type etype = type<T>::erase();
  typesys::universe::instance().get_type(rw._type_index).set_activator(etype.activator);
  return etype;
}

#define register_type(type_name, version) namespace details(type_name) { \
  extern const char name[] = #type_name; \
  using type_reg = typesys::detail::registration<name, version>; \
  static const typesys::detail::registration_witness witness_tag = type_reg::witness(); \
  static const staticinit::evidence<erased_type> actw(&attach_activator<type_name, witness_tag>); \
  }


#define data(type_name) DATA_KIND type_name; \
   register_type(type_name, 1) \
   DATA_KIND type_name : private typesys::detail::data<type_name, details(type_name)::witness_tag, details(type_name)::actw>
#define extension(type_name) DATA_KIND type_name; \
   register_type(type_name, 1) \
   DATA_KIND type_name : typesys::detail::extension<type_name, details(type_name)::witness_tag, details(type_name)::actw
#define of(base_type) , base_type, details(base_type)::witness_tag, details(base_type)::actw>

// ostream operator for printing the versioned_name
inline std::ostream& operator<<(std::ostream& os, const typesys::versioned_name& vn)
{
  return (os << vn.name << "V" << vn.version);
}