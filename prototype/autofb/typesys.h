#pragma once

#include <vector>
#include <unordered_map>
#include <functional>

#define DATA_KIND struct

namespace typesys
{
  using name_t = const char*;
  using version_t = unsigned int;

  enum type_kind {
    TK_BUILTIN,
    TK_DATA
  };

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

  template <name_t N, version_t V>
  struct type_identity
  {
    static constexpr name_t name = N;
    static constexpr version_t version = V;

    static constexpr versioned_name versioned_name()
    {
      return { N, V };
    }
  };

  using type_index = std::uint32_t;

  struct property_info;

  struct type_info
  {
    const type_kind kind;
    const versioned_name name;
    const type_index index;

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
      return register_type_internal(name, TK_DATA);
    }

    const type_info& get_type(name_t name, version_t version)
    {
      // find the type_id in typemap, then chain to get_type(type_id)
      // todo: what to do when not found?
      return get_type(typemap[{name, version}]);
    }

    const type_info& get_type(type_index id) const
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

    inline type_index register_type_internal(const versioned_name& name, type_kind kind)
    {
      // todo: bounds check
      type_index id = static_cast<type_index>(typeinfos.size());

      typeinfos.push_back(type_info{kind, name, id});
      typemap[name] = id;

      return id;
    }

    std::vector<type_info> typeinfos;

    // TODO: make the map lookup actually work by 
    type_map_t typemap;
};

struct property_info
  {
    const char* name;
    const type_index type_index;

    inline const type_info& type() const
    {
      return universe::instance().get_type(type_index);
    }
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
      property(const char* name, property_info&)
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
        return universe::instance().register_type(tid::versioned_name());
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
    inline const type_info& type() const { return universe::instance().get_type(_type_index); }

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
      struct property_info : typesys::property_info
      {
        property_info(const char* name) : detail::property_info{name}
        {
          universe::instance().get_type(rw._name).properties.push_back(*this);
        }

        // TODO: accessors and serializers
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

#define _(type, name) private: property_info<type> name ## _propinfo = property_info<type>(#name); \
  public: __<type> name = __<type>(#name, name ## _propinfo);

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