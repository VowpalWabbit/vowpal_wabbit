// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#define RAPIDJSON_HAS_STDSTRING 1

#include "options.h"
#include "vw.h"
#include "version.h"

#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace VW::config;

template <typename T>
void add_default_value(rapidjson::Value& obj, const T& value, rapidjson::Document::AllocatorType& allocator) = delete;

template <>
void add_default_value<unsigned int>(
    rapidjson::Value& obj, const unsigned int& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<int>(rapidjson::Value& obj, const int& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<int64_t>(
    rapidjson::Value& obj, const int64_t& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<uint64_t>(
    rapidjson::Value& obj, const uint64_t& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<float>(rapidjson::Value& obj, const float& value, rapidjson::Document::AllocatorType& allocator)
{
  if (std::isinf(value))
  {
    rapidjson::Value val;
    val.SetString("inf");
    obj.AddMember("default_value", val, allocator);
  }
  else
  {
    obj.AddMember("default_value", value, allocator);
  }
}

template <>
void add_default_value<double>(
    rapidjson::Value& obj, const double& value, rapidjson::Document::AllocatorType& allocator)
{
  if (std::isinf(value))
  {
    rapidjson::Value val;
    val.SetString("inf");
    obj.AddMember("default_value", val, allocator);
  }
  else
  {
    obj.AddMember("default_value", value, allocator);
  }
}

template <>
void add_default_value<char>(rapidjson::Value& obj, const char& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<std::string>(
    rapidjson::Value& obj, const std::string& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value val;
  val.SetString(value, allocator);
  obj.AddMember("default_value", val, allocator);
}

template <>
void add_default_value<bool>(rapidjson::Value& obj, const bool& value, rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("default_value", value, allocator);
}

template <>
void add_default_value<std::vector<int>>(
    rapidjson::Value& obj, const std::vector<int>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value) { array.PushBack(v, allocator); }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<uint64_t>>(
    rapidjson::Value& obj, const std::vector<uint64_t>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value) { array.PushBack(v, allocator); }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<int64_t>>(
    rapidjson::Value& obj, const std::vector<int64_t>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value) { array.PushBack(v, allocator); }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<float>>(
    rapidjson::Value& obj, const std::vector<float>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value)
  {
    if (std::isinf(v))
    {
      rapidjson::Value val;
      val.SetString("inf");
      array.PushBack(val, allocator);
    }
    else
    {
      array.PushBack(v, allocator);
    }
  }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<double>>(
    rapidjson::Value& obj, const std::vector<double>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value)
  {
    if (std::isinf(v))
    {
      rapidjson::Value val;
      val.SetString("inf");
      array.PushBack(val, allocator);
    }
    else
    {
      array.PushBack(v, allocator);
    }
  }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<char>>(
    rapidjson::Value& obj, const std::vector<char>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value) { array.PushBack(v, allocator); }
  obj.AddMember("default_value", array, allocator);
}

template <>
void add_default_value<std::vector<std::string>>(
    rapidjson::Value& obj, const std::vector<std::string>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (auto& v : value)
  {
    rapidjson::Value val;
    val.SetString(v, allocator);
    array.PushBack(val, allocator);
  }
  obj.AddMember("default_value", array, allocator);
}

struct options_exporter : options_i
{
  options_exporter()
  {
    // must pass an allocator when the object may need to allocate memory
    auto& allocator = _document.GetAllocator();
    // define the _document as an object rather than an array
    _document.SetObject();

    // create a rapidjson array type with similar syntax to std::vector
    rapidjson::Value array(rapidjson::kArrayType);
    _document.AddMember("option_groups", array, allocator);
  }

  template <typename T>
  void inject_type_info_if_t(const std::string& type_name, rapidjson::Value& obj,
      const std::shared_ptr<base_option>& option, rapidjson::Document::AllocatorType& allocator) const
  {
    if (option->m_type_hash == typeid(T).hash_code())
    {
      auto typed = std::dynamic_pointer_cast<typed_option<T>>(option);
      obj.AddMember("type", type_name, allocator);
      obj.AddMember("has_default", typed->default_value_supplied(), allocator);
      if (typed->default_value_supplied())
      {
        // We also need to set the output so VW functions correctly.
        typed->value(typed->default_value(), true);

        add_default_value<T>(obj, typed->default_value(), allocator);
      }
      // There is an implict contract for boolean options to be set to false by default.
      else if (option->m_type_hash == typeid(bool).hash_code())
      {
        auto bool_typed = std::dynamic_pointer_cast<typed_option<bool>>(option);
        bool_typed->value(false, true);
      }

      // We don't want normal VW output in this program.
      if (option->m_name == "quiet")
      {
        auto bool_typed = std::dynamic_pointer_cast<typed_option<bool>>(option);
        bool_typed->value(true, true);
      }
    }
  }

  void inject_type_info(rapidjson::Value& obj, const std::shared_ptr<base_option>& option,
      rapidjson::Document::AllocatorType& allocator) const
  {
    inject_type_info_if_t<uint32_t>("uint32_t", obj, option, allocator);
    inject_type_info_if_t<uint64_t>("uint64_t", obj, option, allocator);
    inject_type_info_if_t<int32_t>("int32_t", obj, option, allocator);
    inject_type_info_if_t<int64_t>("int64_t", obj, option, allocator);
    inject_type_info_if_t<float>("float", obj, option, allocator);
    inject_type_info_if_t<std::string>("string", obj, option, allocator);
    inject_type_info_if_t<bool>("bool", obj, option, allocator);
    inject_type_info_if_t<std::vector<std::string>>("list<string>", obj, option, allocator);
  }

  void add_and_parse(const option_group_definition& group) override
  {
    rapidjson::Document::AllocatorType& allocator = _document.GetAllocator();

    rapidjson::Value group_object(rapidjson::kObjectType);
    rapidjson::Value group_name_value;
    group_name_value.SetString(group.m_name, allocator);
    group_object.AddMember("name", group_name_value, allocator);

    rapidjson::Value options_array(rapidjson::kArrayType);

    for (const auto& option : group.m_options)
    {
      _all_options[option->m_name] = option;
      rapidjson::Value option_obj(rapidjson::kObjectType);

      rapidjson::Value name_value;
      name_value.SetString(option->m_name, allocator);
      option_obj.AddMember("name", name_value, allocator);
      rapidjson::Value help_value;
      help_value.SetString(option->m_help, allocator);
      option_obj.AddMember("help", help_value, allocator);
      if (option->m_short_name != "")
      {
        rapidjson::Value short_name_value;
        short_name_value.SetString(option->m_short_name, allocator);
        option_obj.AddMember("short_name", short_name_value, allocator);
      }
      option_obj.AddMember("keep", option->m_keep, allocator);
      option_obj.AddMember("necessary", option->m_necessary, allocator);
      if (option->m_type_hash == typed_option<std::string>::type_hash())
      {
        const auto str_option = std::dynamic_pointer_cast<typed_option<std::string>>(option);
        if (!str_option->one_of().empty())
        {
          rapidjson::Value one_of_array(rapidjson::kArrayType);
          for (const auto& str : str_option->one_of())
          {
            rapidjson::Value one_of_val;
            one_of_val.SetString(str, allocator);
            one_of_array.PushBack(one_of_val, allocator);
          }
          option_obj.AddMember("one_of", one_of_array, allocator);
        }
      }
      else if (option->m_type_hash == typed_option<int>::type_hash())
      {
        const auto int_option = std::dynamic_pointer_cast<typed_option<int>>(option);
        if (!int_option->one_of().empty())
        {
          rapidjson::Value one_of_array(rapidjson::kArrayType);
          for (const int i : int_option->one_of())
          {
            rapidjson::Value one_of_val;
            one_of_val.SetInt(i);
            one_of_array.PushBack(one_of_val, allocator);
          }
          option_obj.AddMember("one_of", one_of_array, allocator);
        }
      }
      inject_type_info(option_obj, option, allocator);
      options_array.PushBack(option_obj, allocator);
    }

    group_object.AddMember("options", options_array, allocator);
    _document["option_groups"].PushBack(group_object, allocator);
  }

  bool add_parse_and_check_necessary(const option_group_definition& group) override
  {
    add_and_parse(group);
    return false;
  }

  bool was_supplied(const std::string&) const override { return false; }

  void tint(const std::string&) override {}

  void reset_tint() override {}

  std::string help(const std::vector<std::string>&) const override
  {
    throw std::runtime_error{"This function is not defined."};
  }

  void check_unregistered(VW::io::logger& /* logger */) override {}

  std::vector<std::shared_ptr<base_option>> get_all_options() override
  {
    throw std::runtime_error{"This function is not defined."};
  }

  std::vector<std::shared_ptr<const base_option>> get_all_options() const override
  {
    throw std::runtime_error{"This function is not defined."};
  }

  std::shared_ptr<base_option> get_option(const std::string& name) override
  {
    const auto it = _all_options.find(name);
    if (it != _all_options.end()) { return it->second; }

    throw std::out_of_range(name + " was not found.");
  }

  std::shared_ptr<const base_option> get_option(const std::string& name) const override
  {
    const auto it = _all_options.find(name);
    if (it != _all_options.end()) { return it->second; }

    throw std::out_of_range(name + " was not found.");
  }

  std::map<std::string, std::vector<option_group_definition>> get_collection_of_options() const override
  {
    throw std::runtime_error{"This function is not defined."};
  }
  void insert(const std::string&, const std::string&) override
  {
    throw std::runtime_error{"This function is not defined."};
  }
  void replace(const std::string&, const std::string&) override
  {
    throw std::runtime_error{"This function is not defined."};
  }
  std::vector<std::string> get_positional_tokens() const override { return {}; }

  std::map<std::string, std::shared_ptr<base_option>> _all_options;
  rapidjson::Document _document;
};

int main(int argc, char* argv[])
{
  options_exporter exporter;
  auto* vw = VW::initialize(exporter);

  auto& allocator = exporter._document.GetAllocator();
  rapidjson::Value version_object(rapidjson::kObjectType);
  rapidjson::Value version_text;
  version_text.SetString(VW::version.to_string(), allocator);
  version_object.AddMember("version", version_text, allocator);
  rapidjson::Value git_commit_text;
  git_commit_text.SetString(VW::git_commit, allocator);
  version_object.AddMember("git_commit", git_commit_text, allocator);
  exporter._document.AddMember("version_info", version_object, allocator);

  rapidjson::StringBuffer strbuf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
  exporter._document.Accept(writer);

  std::cout << strbuf.GetString() << std::endl;

  delete vw;

  return 0;
}
