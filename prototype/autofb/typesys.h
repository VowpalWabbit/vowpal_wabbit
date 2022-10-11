#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#include "typeerase.h"

#define DATA_KIND struct

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
      return universe::instance().register_type(versioned_name());
    }
  };

  struct property_info
{
  const char* name;
  const erased_type etype;

  property_info(const char* name, erased_type etype) : name(name), etype(etype) {};
  property_info(property_info& other) = default;
};

  struct type_info
  {
    const versioned_name name;
    const type_index index;

    type_info(versioned_name name, type_index index) : name{name}, index{index} {}
    type_info(type_info& other) = default;

    inline void add_property(property_info pi) { properties.push_back(property_info{pi}); }
    
    using props_iter = std::vector<property_info>::const_iterator;

    inline props_iter props_begin() const { return properties.cbegin(); };
    inline props_iter props_end() const { return properties.cend(); };

  private:
    std::vector<property_info> properties;
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

    inline type_index register_type(name_t name, version_t version) 
    {
      return register_type({name, version});
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

    template <typename T, typename = std::enable_if<std::is_literal_type<T>::value>::type>
    inline type_index register_builtin() 
    {
      register_type_internal({typeid(T).name(), 0}, TK_BUILTIN); 
    }

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
      property(const char* name, const property_info&)
      {
        //TODO: register property location with reflector
      }

      // TODO: Rest of Rule of N

      void operator=(const T& value) { _value = value; }

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

    template <name_t N, version_t V>
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

  template <typename concrete_t, const registration_witness& rw>
  DATA_KIND data
  {
    public:
      //static const int version = rw._name.version;

    public:
      data()
      {}

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
          std::cout << "in prop witness" << std::endl;
          universe::instance().get_type(rw._name).add_property(info);
          //props.push_back(property_info(info));
        }
      };
  };

  template <typename concrete_t, const registration_witness& rw, typename base_t>
  DATA_KIND extension : private data<concrete_t, rw>, public base_t
  {
    //using data<concrete_t, rw>::version;

    using data<concrete_t, rw>::__;
    using data<concrete_t, rw>::property_info;
  };

} // namespace detail
} // namespace typesys

#define _(type, name) private: static property_witness< ## type ## >& name ## _propwitness() { static property_witness<type> pi{#name}; return pi; } \
  public: __<type> name = __<type>(#name, name ## _propwitness().info);

// TODO: Versions
#define details(type_name) __ ## type_name ## _details

#define register_type(type_name, version) namespace details(type_name) { \
  extern const char name[] = #type_name; \
  using type_reg = detail::registration<name, version>; \
  static const detail::registration_witness witness = type_reg::witness(); }

#define data(type_name) register_type(type_name, 1) \
   DATA_KIND type_name : private detail::data<type_name, details(type_name)::witness>
#define extension(type_name) register_type(type_name, 1) \
   DATA_KIND type_name : detail::extension<type_name, details(type_name)::witness
#define of(base_type) , base_type>