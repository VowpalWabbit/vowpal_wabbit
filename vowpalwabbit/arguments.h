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

    static size_t type_hash() {
      return typeid(T).hash_code();
    }

    typed_argument& default_value(T value) {
      m_default_value = std::make_shared<T>(value);
      return *this;
    }

    bool default_value_supplied() {
      return m_default_value.get() != nullptr;
    }

    T default_value() {
      return m_default_value ? *m_default_value : T();
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

    bool value_supplied() {
      return m_value.get() != nullptr;
    }

    typed_argument& value(T& value) {
      m_value = std::make_shared<T>(value);
      return *this;
    }

    T value(){
      return m_value ? *m_value : T();
    }

    std::vector<T*> m_locations;

  private:
    // Would prefer to use std::optional (C++17) here but we are targeting C++11
    std::shared_ptr<T> m_value{ nullptr };
    std::shared_ptr<T> m_default_value{ nullptr };
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

    virtual void merge(arguments_i* other) = 0;

    virtual std::vector<std::shared_ptr<base_argument>>& get_all_args() = 0;
    virtual base_argument& get_arg(std::string key) = 0;

    template <typename T>
    typed_argument<T>& get_typed_arg(std::string key) {
      base_argument& base = get_arg(key);
      if (base.m_type_hash != typed_argument<T>::type_hash()) {
        throw std::bad_cast();
      }

      return dynamic_cast<typed_argument<T>&>(base);
    }

    // Will throw if any options were supplied that do not having a matching argument specification.
    virtual void check_unregistered() = 0;

    virtual ~arguments_i() {}
  };

  struct arguments_serializer_i {
    virtual void add(base_argument& argument) = 0;
    virtual std::string str() = 0;
    virtual const char* data() = 0;
    virtual size_t size() = 0;
  };

  template<typename T>
  bool operator==(typed_argument<T>& lhs, typed_argument<T>& rhs) {
    return lhs.m_name == rhs.m_name
      && lhs.m_type_hash == rhs.m_type_hash
      && lhs.m_help == rhs.m_help
      && lhs.m_short_name == rhs.m_short_name
      && lhs.m_keep == rhs.m_keep
      && lhs.default_value() == rhs.default_value();
  }

  template<typename T>
  bool operator!=(typed_argument<T>& lhs, typed_argument<T>& rhs) {
    return !(lhs == rhs);
  }

  bool operator==(const base_argument& lhs, const base_argument& rhs);
  bool operator!=(const base_argument& lhs, const base_argument& rhs);
}
