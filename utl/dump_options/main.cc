// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/memory.h"
#define RAPIDJSON_HAS_STDSTRING 1

#include "vw/config/help_formatter.h"
#include "vw/config/options.h"
#include "vw/core/version.h"
#include "vw/core/vw.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <memory>

using namespace VW::config;

template <typename T>
void add_default_value(rapidjson::Value& obj, const T& value, rapidjson::Document::AllocatorType& allocator)
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
  else { obj.AddMember("default_value", value, allocator); }
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
void add_default_value<std::vector<std::string>>(
    rapidjson::Value& obj, const std::vector<std::string>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value array(rapidjson::kArrayType);
  for (const auto& v : value)
  {
    rapidjson::Value val;
    val.SetString(v, allocator);
    array.PushBack(val, allocator);
  }
  obj.AddMember("default_value", array, allocator);
}

template <typename T>
void add_one_of(rapidjson::Value& obj, const std::set<T>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value one_of_array(rapidjson::kArrayType);
  for (const auto& val : value) { one_of_array.PushBack(val, allocator); }
  obj.AddMember("one_of", one_of_array, allocator);
}

template <>
void add_one_of<std::string>(
    rapidjson::Value& obj, const std::set<std::string>& value, rapidjson::Document::AllocatorType& allocator)
{
  rapidjson::Value one_of_array(rapidjson::kArrayType);
  for (const auto& val : value)
  {
    rapidjson::Value one_of_val;
    one_of_val.SetString(val, allocator);
    one_of_array.PushBack(one_of_val, allocator);
  }
  obj.AddMember("one_of", one_of_array, allocator);
}

template <>
void add_one_of<std::vector<std::string>>(rapidjson::Value& obj, const std::set<std::vector<std::string>>& value,
    rapidjson::Document::AllocatorType& allocator)
{
  THROW("not supported");
}

template <typename T>
void inject_typed_info(const typed_option<T>& option, const std::string& type_name, rapidjson::Value& obj,
    rapidjson::Document::AllocatorType& allocator)
{
  obj.AddMember("type", type_name, allocator);
  obj.AddMember("has_default", option.default_value_supplied(), allocator);
  if (option.default_value_supplied()) { add_default_value<T>(obj, option.default_value(), allocator); }
  if (!option.one_of().empty()) { add_one_of<T>(obj, option.one_of(), allocator); }
}

struct type_info_injector : typed_option_visitor
{
  rapidjson::Value& m_obj;
  rapidjson::Document::AllocatorType& m_allocator;

  type_info_injector(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator)
      : m_obj(obj), m_allocator(allocator)
  {
  }

  void visit(typed_option<uint32_t>& option) override { inject_typed_info(option, "uint", m_obj, m_allocator); }
  void visit(typed_option<uint64_t>& option) override { inject_typed_info(option, "uint", m_obj, m_allocator); }
  void visit(typed_option<int64_t>& option) override { inject_typed_info(option, "int", m_obj, m_allocator); }
  void visit(typed_option<int32_t>& option) override { inject_typed_info(option, "int", m_obj, m_allocator); }
  void visit(typed_option<bool>& option) override { inject_typed_info(option, "bool", m_obj, m_allocator); }
  void visit(typed_option<float>& option) override { inject_typed_info(option, "float", m_obj, m_allocator); }
  void visit(typed_option<std::string>& option) override { inject_typed_info(option, "str", m_obj, m_allocator); }
  void visit(typed_option<std::vector<std::string>>& option) override
  {
    inject_typed_info(option, "list[str]", m_obj, m_allocator);
  }
};

struct json_help_formatter : VW::config::help_formatter
{
  rapidjson::Document document;

  json_help_formatter(rapidjson::Document&& doc) : document(std::move(doc)) {}

  std::string format_help(const std::vector<option_group_definition>& groups) override
  {
    // must pass an allocator when the object may need to allocate memory
    auto& allocator = document.GetAllocator();

    // create a rapidjson array type with similar syntax to std::vector
    rapidjson::Value array(rapidjson::kArrayType);
    document.AddMember("option_groups", array, allocator);

    for (const auto& group : groups)
    {
      rapidjson::Value group_object(rapidjson::kObjectType);
      rapidjson::Value group_name_value;
      group_name_value.SetString(group.m_name, allocator);
      group_object.AddMember("name", group_name_value, allocator);

      rapidjson::Value options_array(rapidjson::kArrayType);

      for (const auto& option : group.m_options)
      {
        rapidjson::Value option_obj(rapidjson::kObjectType);

        rapidjson::Value name_value;
        name_value.SetString(option->m_name, allocator);
        option_obj.AddMember("name", name_value, allocator);
        rapidjson::Value help_value;
        help_value.SetString(option->m_help, allocator);
        option_obj.AddMember("help", help_value, allocator);
        if (!option->m_short_name.empty())
        {
          rapidjson::Value short_name_value;
          short_name_value.SetString(option->m_short_name, allocator);
          option_obj.AddMember("short_name", short_name_value, allocator);
        }
        option_obj.AddMember("keep", option->m_keep, allocator);
        option_obj.AddMember("necessary", option->m_necessary, allocator);
        option_obj.AddMember("experimental", option->m_experimental, allocator);
        type_info_injector injector(option_obj, allocator);
        option->accept(injector);
        options_array.PushBack(option_obj, allocator);
      }

      group_object.AddMember("options", options_array, allocator);
      document["option_groups"].PushBack(group_object, allocator);
    }
    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    document.Accept(writer);
    return strbuf.GetString();
  }
};

int main(int argc, char* argv[])
{
  std::vector<std::string> args{"--quiet"};
  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));

  rapidjson::Document doc;
  auto& allocator = doc.GetAllocator();

  // define the document as an object rather than an array
  doc.SetObject();

  rapidjson::Value version_object(rapidjson::kObjectType);
  rapidjson::Value version_text;
  version_text.SetString(VW::VERSION.to_string(), allocator);
  version_object.AddMember("version", version_text, allocator);
  rapidjson::Value git_commit_text;
  git_commit_text.SetString(VW::GIT_COMMIT, allocator);
  version_object.AddMember("git_commit", git_commit_text, allocator);
  doc.AddMember("version_info", version_object, allocator);

  json_help_formatter formatter(std::move(doc));
  std::cout << formatter.format_help(vw->options->get_all_option_group_definitions()) << std::endl;

  return 0;
}
