#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include <memory>

namespace VW {

  struct base_argument {
    base_argument(std::string name, size_t type_hash)
      : m_name(name), m_type_hash(type_hash)
    {}

    std::string m_name;
    size_t m_type_hash;
    std::string m_help = "";
    std::string m_short_name = "";
    bool m_keep = false;

    virtual ~base_argument() {}
  };

  template<typename T>
  struct typed_argument : base_argument {
    typed_argument(std::string name, T* location)
      : base_argument(name, typeid(T).hash_code()) {
      m_locations.push_back(location);
    }

    typed_argument& default_value(T value) {
      m_default_supplied = true;
      m_default_value = value;
      return *this;
    }

    typed_argument& short_name(std::string short_name) {
      m_short_name = short_name;
      return *this;
    }

    typed_argument& help(std::string help) {
      m_help = help; return *this;
    }

    typed_argument& keep(bool keep = true) {
      m_keep = keep; return *this;
    }

    bool m_default_supplied = false;
    T m_default_value;
    std::vector<T*> m_locations;
  };

  template<typename T>
  typed_argument<T> make_typed_arg(std::string name, T* location) {
    return typed_argument<T>(name, location);
  }

  struct argument_group_definition {
    argument_group_definition(std::string name)
      : m_name(name)
    {}

    template<typename T>
    void add(typed_argument<T> op) {
      m_arguments.push_back(std::make_shared<typed_argument<T>>(op));
    }

    template<typename T>
    argument_group_definition& operator()(typed_argument<T> op) {
      add(op);
      return *this;
    }

    std::string m_name;
    std::vector<std::shared_ptr<base_argument>> m_arguments;
  };

  struct arguments_i {
    virtual void add_and_parse(argument_group_definition group) = 0;
    virtual bool was_supplied(std::string key) = 0;
    virtual std::string help() = 0;
    virtual std::string get_kept() = 0;

    // Will throw if any options were supplied that do not having a matching argument specification.
    virtual void check_unregistered() = 0;

    virtual ~arguments_i() {}
  };

  template<typename T>
  bool operator==(const typed_argument<T>& lhs, const typed_argument<T>& rhs) {
    return lhs.m_name == rhs.m_name
      && lhs.m_type_hash == rhs.m_type_hash
      && lhs.m_help == rhs.m_help
      && lhs.m_short_name == rhs.m_short_name
      && lhs.m_keep == rhs.m_keep
      && lhs.m_default_value == rhs.m_default_value;
  }

  template<typename T>
  bool operator!=(const typed_argument<T>& lhs, const typed_argument<T>& rhs) {
    return !(lhs == rhs);
  }

  bool operator==(const base_argument& lhs, const base_argument& rhs);
  bool operator!=(const base_argument& lhs, const base_argument& rhs);
}
