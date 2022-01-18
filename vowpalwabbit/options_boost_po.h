// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <memory>
#include <vector>
#include <sstream>
#include <set>
#include <algorithm>
#include <string>
#include <unordered_map>

#include "options.h"
#include "options_types.h"
#include "vw_exception.h"

// Boost Program Options requires that all types that have a default option are ostreamable
namespace std
{
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  for (auto const& item : vec) { os << item << ", "; }
  return os;
}

// The std::vector<bool> specialization does not support ref access, so we need to handle this differently.
std::ostream& operator<<(std::ostream& os, const std::vector<bool>& vec);
}  // namespace std

template std::ostream& std::operator<<<int>(std::ostream&, const std::vector<int>&);
template std::ostream& std::operator<<<char>(std::ostream&, const std::vector<char>&);
template std::ostream& std::operator<<<std::string>(std::ostream&, const std::vector<std::string>&);
template std::ostream& std::operator<<<float>(std::ostream&, const std::vector<float>&);

namespace VW
{
namespace config
{

struct boost_options_description_adder;

struct options_boost_po : public options_i
{
  options_boost_po(int argc, char** argv) : options_boost_po(std::vector<std::string>(argv + 1, argv + argc)) {}

  options_boost_po(const std::vector<std::string>& args) : m_command_line(args) {}

  options_boost_po(options_boost_po&) = delete;
  options_boost_po& operator=(options_boost_po&) = delete;

  void add_and_parse(const option_group_definition& group) override;
  bool add_parse_and_check_necessary(const option_group_definition& group) override;
  bool was_supplied(const std::string& key) const override;
  std::string help(const std::vector<std::string>& enabled_reductions) const override;
  void check_unregistered(VW::io::logger& logger) override;
  std::vector<std::shared_ptr<base_option>> get_all_options() override;
  std::vector<std::shared_ptr<const base_option>> get_all_options() const override;
  std::shared_ptr<base_option> get_option(const std::string& key) override;
  std::shared_ptr<const base_option> get_option(const std::string& key) const override;

  void tint(const std::string& reduction_name) override { m_current_reduction_tint = reduction_name; }

  void reset_tint() override { m_current_reduction_tint = m_default_tint; }

  void insert(const std::string& key, const std::string& value) override
  {
    m_command_line.push_back("--" + key);
    if (!value.empty()) { m_command_line.push_back(value); }
  }

  // Note: does not work for vector options.
  void replace(const std::string& key, const std::string& value) override
  {
    auto full_key = "--" + key;
    auto it = std::find(m_command_line.begin(), m_command_line.end(), full_key);

    // Not found, insert instead.
    if (it == m_command_line.end())
    {
      insert(key, value);
      return;
    }

    // Check if it is the final option or the next option is not a value.
    if (it + 1 == m_command_line.end() || (*(it + 1)).find("--") != std::string::npos)
    { THROW(key + " option does not have a value."); }

    // Actually replace the value.
    *(it + 1) = value;
  }

  std::vector<std::string> get_positional_tokens() const override
  {
    po::positional_options_description p;
    p.add("__positional__", -1);
    auto copied_description = master_description;
    copied_description.add_options()("__positional__", po::value<std::vector<std::string>>()->composing(), "");
    po::parsed_options pos = po::command_line_parser(m_command_line)
                                 .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
                                 .options(copied_description)
                                 .allow_unregistered()
                                 .positional(p)
                                 .run();

    po::variables_map vm;
    po::store(pos, vm);

    if (vm.count("__positional__") != 0) { return vm["__positional__"].as<std::vector<std::string>>(); }
    return std::vector<std::string>();
  }

  std::map<std::string, std::vector<option_group_definition>> get_collection_of_options() const override
  {
    return m_option_group_dic;
  }

  const std::string m_default_tint = "general";

private:
  void add_to_option_group_collection(const option_group_definition& group);
  void internal_add_and_parse(const option_group_definition& group);

  friend struct boost_options_description_adder;

  // Collection that tracks for now
  // setup_function_id (str) -> list of option_group_definition
  std::map<std::string, std::vector<option_group_definition>> m_option_group_dic;

  std::string m_current_reduction_tint = m_default_tint;

  std::map<std::string, std::shared_ptr<base_option>> m_options;

  std::vector<std::string> m_command_line;

  std::map<std::string, std::stringstream> m_help_stringstream;

  std::set<std::string> m_added_help_group_names;

  // All options that were supplied on the command line.
  std::set<std::string> m_supplied_options;

  // Used the ignore values that get incorrectly interpreted as options.
  std::set<std::string> m_ignore_supplied;

  po::options_description master_description;

  // All options that a description was provided for.
  std::set<std::string> m_defined_options;

  std::set<std::string> m_reachable_options;
  std::unordered_map<std::string, std::vector<std::set<std::string>>> m_dependent_necessary_options;
};

}  // namespace config
}  // namespace VW
