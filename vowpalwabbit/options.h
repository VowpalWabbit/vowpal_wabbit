#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include <memory>

struct base_option {
  base_option(std::string name, size_t type_hash)
    : m_name(name), m_type_hash(type_hash)
  {}

  size_t m_type_hash;
  std::string m_name;
  std::string m_help = "";
  std::string m_short_name = "";
  bool m_keep = false;

  virtual ~base_option() {}
};

template<typename T>
struct typed_option : base_option {
  explicit typed_option(std::string name, T* location)
    : base_option(name, typeid(T).hash_code()) {
    m_locations.push_back(location);
  }

  typed_option& default_value(T value) {
    m_default_supplied = true;
    m_default_value = value;
    return *this;
  }

  typed_option& short_name(std::string short_name) { m_short_name = short_name; return *this; }
  typed_option& help(std::string help) { m_help = help; return *this; }
  typed_option& keep(bool keep = true) { m_keep = keep; return *this; }

  bool m_default_supplied = false;
  T m_default_value;
  std::vector<T*> m_locations;
};

template<typename T>
typed_option<T> make_typed_option(std::string name, T* location) {
  return typed_option<T>(name, location);
}

template<typename T>
bool operator==(const typed_option<T>& lhs, const typed_option<T>& rhs) {
  return lhs.m_name == rhs.m_name
    && lhs.m_type_hash == rhs.m_type_hash
    && lhs.m_help == rhs.m_help
    && lhs.m_short_name == rhs.m_short_name
    && lhs.m_keep == rhs.m_keep
    && lhs.m_default_value == rhs.m_default_value;
}

template<typename T>
bool operator!=(const typed_option<T>& lhs, const typed_option<T>& rhs) {
  return !(lhs == rhs);
}

struct option_group_definition {
  option_group_definition(std::string name) : m_name(name) {}

  template<typename T>
  void add(typed_option<T> op) { m_options.push_back(std::make_shared<typed_option<T>>(op)); }

  template<typename T>
  option_group_definition& operator()(typed_option<T> op) {
    add(op);
    return *this;
  }

  std::string m_name;
  std::vector<std::shared_ptr<base_option>> m_options;
};

struct options_i {
  virtual void add_and_parse(option_group_definition group) = 0;
  virtual bool was_supplied(std::string key) = 0;
  virtual std::string help() = 0;
  virtual std::string get_kept() = 0;

  virtual ~options_i() {}
};
