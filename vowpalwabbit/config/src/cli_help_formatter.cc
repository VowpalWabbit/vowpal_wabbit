// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/cli_help_formatter.h"

#include "vw/common/text_utils.h"
#include "vw/config/options.h"

#include <fmt/format.h>

#include <sstream>
#include <string>

using namespace VW::config;

// Returns the one of values as a comma separated string or empty if there are none.
std::string one_of_string(base_option& opt)
{
  std::string ret_val = "";
  class extractor : public typed_option_visitor
  {
  public:
    std::string& m_ret_val;
    extractor(std::string& ret_val) : m_ret_val(ret_val) {}

    void visit(typed_option<uint32_t>& option) override
    {
      if (!option.one_of().empty()) { m_ret_val = fmt::format("{}", fmt::join(option.one_of(), ", ")); }
    };
    void visit(typed_option<uint64_t>& option) override
    {
      if (!option.one_of().empty()) { m_ret_val = fmt::format("{}", fmt::join(option.one_of(), ", ")); }
    };
    void visit(typed_option<int64_t>& option) override
    {
      if (!option.one_of().empty()) { m_ret_val = fmt::format("{}", fmt::join(option.one_of(), ", ")); }
    };
    void visit(typed_option<int32_t>& option) override
    {
      if (!option.one_of().empty()) { m_ret_val = fmt::format("{}", fmt::join(option.one_of(), ", ")); }
    };
    void visit(typed_option<bool>& /*option*/) override{};
    void visit(typed_option<float>& /*option*/) override{};
    void visit(typed_option<std::string>& option) override
    {
      if (!option.one_of().empty()) { m_ret_val = fmt::format("{}", fmt::join(option.one_of(), ", ")); }
    };
    void visit(typed_option<std::vector<std::string>>& /*option*/) override{};
  };

  extractor ex(ret_val);
  opt.accept(ex);
  return ret_val;
}

// Returns the one of values as a comma separated string or empty if there are none.
std::string type_string(base_option& opt)
{
  std::string ret_val = "";
  class extractor : public typed_option_visitor
  {
  public:
    std::string& m_ret_val;
    extractor(std::string& ret_val) : m_ret_val(ret_val) {}

    void visit(typed_option<uint32_t>& /*option*/) override { m_ret_val = "uint"; };
    void visit(typed_option<uint64_t>& /*option*/) override { m_ret_val = "uint"; };
    void visit(typed_option<int64_t>& /*option*/) override { m_ret_val = "int"; };
    void visit(typed_option<int32_t>& /*option*/) override { m_ret_val = "int"; };
    void visit(typed_option<bool>& /*option*/) override { m_ret_val = "bool"; };
    void visit(typed_option<float>& /*option*/) override { m_ret_val = "float"; };
    void visit(typed_option<std::string>& /*option*/) override { m_ret_val = "str"; };
    void visit(typed_option<std::vector<std::string>>& /*option*/) override { m_ret_val = "list[str]"; };
  };

  extractor ex(ret_val);
  opt.accept(ex);
  return ret_val;
}

// Returns the one of values as a comma separated string or empty if there are none.
std::string default_value_string(base_option& opt)
{
  std::string ret_val = "";
  class extractor : public typed_option_visitor
  {
  public:
    std::string& m_ret_val;
    extractor(std::string& ret_val) : m_ret_val(ret_val) {}

    void visit(typed_option<uint32_t>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<uint64_t>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<int64_t>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<int32_t>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<bool>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<float>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<std::string>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", option.default_value()); }
    };
    void visit(typed_option<std::vector<std::string>>& option) override
    {
      if (option.default_value_supplied()) { m_ret_val = fmt::format("{}", fmt::join(option.default_value(), ", ")); }
    };
  };

  extractor ex(ret_val);
  opt.accept(ex);
  return ret_val;
}

std::vector<std::string> split_string_by_newline(const std::string& str)
{
  auto result = std::vector<std::string>{};
  auto ss = std::stringstream{str};

  for (std::string line; std::getline(ss, line, '\n');) { result.push_back(line); }

  return result;
}

constexpr size_t LEFT_COL_WIDTH = 40;
constexpr size_t RIGHT_COL_WIDTH = 60;
static const std::string INDENT = "    ";

std::string cli_help_formatter::format_help(const std::vector<option_group_definition>& groups)
{
  std::stringstream overall_ss;
  for (const auto& group : groups)
  {
    overall_ss << group.m_name << ":\n";
    for (const auto& option : group.m_options)
    {
      if (option->m_hidden_from_help) { continue; }
      std::stringstream ss_option_name;
      if (!option->m_short_name.empty()) { ss_option_name << "-" << option->m_short_name << ", "; }
      ss_option_name << "--" << option->m_name;
      auto type = type_string(*option);
      if (type == "list[str]") { ss_option_name << " args..."; }
      else if (type != "bool") { ss_option_name << " arg"; }
      std::string option_name_str = ss_option_name.str();

      std::stringstream ss_description;
      ss_description << option->m_help;

      ss_description << " (type: " << type_string(*option);

      if (!default_value_string(*option).empty()) { ss_description << ", default: " << default_value_string(*option); }
      auto one_of = one_of_string(*option);
      if (!one_of.empty()) { ss_description << ", choices {" << one_of << "}"; }
      if (option->m_keep) { ss_description << ", keep"; }
      if (option->m_necessary) { ss_description << ", necessary"; }
      if (option->m_experimental) { ss_description << ", experimental"; }
      ss_description << ")";

      auto help_lines = split_string_by_newline(VW::wrap_text(ss_description.str(), RIGHT_COL_WIDTH));

      size_t help_line_to_start_at = 0;
      if (option_name_str.size() > LEFT_COL_WIDTH) { overall_ss << INDENT << option_name_str << "\n"; }
      else
      {
        auto first_line_padding = option_name_str.size() < LEFT_COL_WIDTH ? LEFT_COL_WIDTH - option_name_str.size() : 0;
        overall_ss << INDENT << option_name_str << std::string(first_line_padding, ' ') << help_lines[0] << "\n";
        help_line_to_start_at = 1;
      }

      for (size_t i = help_line_to_start_at; i < help_lines.size(); i++)
      {
        overall_ss << INDENT << std::string(LEFT_COL_WIDTH, ' ') << help_lines[i] << "\n";
      }
    }
  }

  return overall_ss.str();
}