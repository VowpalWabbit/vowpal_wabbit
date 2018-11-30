#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include <memory>

struct base_parameter {
  base_parameter(std::string name, size_t type_hash)
    : m_name(name), m_type_hash(type_hash)
  {}

  size_t m_type_hash;
  std::string m_name;
  std::string m_help = "";
  std::string m_short_name = "";
  bool m_keep = false;

  virtual ~base_parameter() {}
};

template<typename T>
struct typed_parameter : base_parameter {
  explicit typed_parameter(std::string name, T* location)
    : base_parameter(name, typeid(T).hash_code()) {
    m_locations.push_back(location);
  }

  typed_parameter& default_value(T value) {
    m_default_supplied = true;
    m_default_value = value;
    return *this;
  }

  typed_parameter& short_name(std::string short_name) { m_short_name = short_name; return *this; }
  typed_parameter& help(std::string help) { m_help = help; return *this; }
  typed_parameter& keep(bool keep = true) { m_keep = keep; return *this; }

  bool m_default_supplied = false;
  T m_default_value;
  std::vector<T*> m_locations;
};

template<typename T>
typed_parameter<T> make_typed_option(std::string name, T* location) {
  return typed_parameter<T>(name, location);
}

template<typename T>
bool operator==(const typed_parameter<T>& lhs, const typed_parameter<T>& rhs) {
  return lhs.m_name == rhs.m_name
    && lhs.m_type_hash == rhs.m_type_hash
    && lhs.m_help == rhs.m_help
    && lhs.m_short_name == rhs.m_short_name
    && lhs.m_keep == rhs.m_keep
    && lhs.m_default_value == rhs.m_default_value;
}

template<typename T>
bool operator!=(const typed_parameter<T>& lhs, const typed_parameter<T>& rhs) {
  return !(lhs == rhs);
}

struct argument_group_definition {
  argument_group_definition(std::string name) : m_name(name) {}

  template<typename T>
  void add(typed_parameter<T> op) { m_parameters.push_back(std::make_shared<typed_parameter<T>>(op)); }

  template<typename T>
  argument_group_definition& operator()(typed_parameter<T> op) {
    add(op);
    return *this;
  }

  std::string m_name;
  std::vector<std::shared_ptr<base_parameter>> m_parameters;
};

struct arguments_i {
  virtual void add_and_parse(argument_group_definition group) = 0;
  virtual bool was_supplied(std::string key) = 0;
  virtual std::string help() = 0;
  virtual std::string get_kept() = 0;

  virtual ~arguments_i() {}
};
