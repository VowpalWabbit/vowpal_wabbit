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

#include "options.h"
#include "options_types.h"
#include "vw_exception.h"

// Boost Program Options requires that all types that have a default option are ostreamable
namespace std
{
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  for (auto const& item : vec)
  {
    os << item << ", ";
  }
  return os;
}
}  // namespace std

template std::ostream& std::operator<<<int>(std::ostream&, const std::vector<int>&);
template std::ostream& std::operator<<<char>(std::ostream&, const std::vector<char>&);
template std::ostream& std::operator<<<std::string>(std::ostream&, const std::vector<std::string>&);
template std::ostream& std::operator<<<float>(std::ostream&, const std::vector<float>&);
template std::ostream& std::operator<<<bool>(std::ostream&, const std::vector<bool>&);

namespace VW
{
namespace config
{
struct options_boost_po : public options_i
{
  options_boost_po(int argc, char** argv) : options_boost_po(std::vector<std::string>(argv + 1, argv + argc)) {}

  options_boost_po(std::vector<std::string> args) : m_command_line(args) {}

  options_boost_po(options_boost_po&) = delete;
  options_boost_po& operator=(options_boost_po&) = delete;

  virtual void add_and_parse(const option_group_definition& group) override;
  virtual bool was_supplied(const std::string& key) override;
  virtual std::string help() override;
  virtual void check_unregistered() override;
  virtual std::vector<std::shared_ptr<base_option>> get_all_options() override;
  virtual std::shared_ptr<base_option> get_option(const std::string& key) override;

  virtual void insert(const std::string& key, const std::string& value) override
  {
    m_command_line.push_back("--" + key);
    if (value != "")
    {
      m_command_line.push_back(value);
    }
  }

  // Note: does not work for vector options.
  virtual void replace(const std::string& key, const std::string& value) override
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
    {
      THROW(key + " option does not have a value.");
    }

    // Actually replace the value.
    *(it + 1) = value;
  }

  // key must reference an option previously defined.
  bool try_get_positional_option_token(const std::string& key, std::string& token, int position)
  {
    po::positional_options_description p;
    p.add(key.c_str(), position);
    po::parsed_options pos = po::command_line_parser(m_command_line)
                                 .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
                                 .options(master_description)
                                 .allow_unregistered()
                                 .positional(p)
                                 .run();

    auto it = std::find_if(pos.options.begin(), pos.options.end(),
        [&key](boost::program_options::option& option) { return option.string_key == key; });

    if (it != pos.options.end() && (*it).value.size() > 0)
    {
      token = (*it).value.at(0);
      return true;
    }

    return false;
  }

 private:
  template <typename T>
  typename po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<T>>& opt);

  template <typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt);

  template <typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<T>>& opt);

  template <typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt);

  template <typename T>
  po::typed_value<std::vector<T>>* add_notifier(
      std::shared_ptr<typed_option<T>>& opt, po::typed_value<std::vector<T>>* po_value);

  template <typename T>
  po::typed_value<std::vector<T>>* add_notifier(
      std::shared_ptr<typed_option<std::vector<T>>>& opt, po::typed_value<std::vector<T>>* po_value);

  template <typename T>
  bool add_if_t(std::shared_ptr<base_option> opt, po::options_description& options_description);

  void add_to_description(std::shared_ptr<base_option> opt, po::options_description& options_description);

  template <typename TTypes>
  void add_to_description_impl(std::shared_ptr<base_option> opt, po::options_description& options_description)
  {
    if (add_if_t<typename TTypes::head>(opt, options_description))
    {
      return;
    }
    add_to_description_impl<typename TTypes::tail>(opt, options_description);
  }

  template <typename T>
  void add_to_description(std::shared_ptr<typed_option<T>> opt, po::options_description& options_description);

 private:
  std::map<std::string, std::shared_ptr<base_option>> m_options;

  std::vector<std::string> m_command_line;

  std::stringstream m_help_stringstream;

  // All options that were supplied on the command line.
  std::set<std::string> m_supplied_options;

  // Used the ignore values that get incorrectly interpreted as options.
  std::set<std::string> m_ignore_supplied;

  po::options_description master_description;

  // All options that a description was provided for.
  std::set<std::string> m_defined_options;
};

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::get_base_boost_value(std::shared_ptr<typed_option<T>>& opt)
{
  auto value = po::value<std::vector<T>>();

  if (opt->default_value_supplied())
  {
    value->default_value({opt->default_value()});
  }

  return add_notifier(opt, value)->composing();
}

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::get_base_boost_value(
    std::shared_ptr<typed_option<std::vector<T>>>& opt)
{
  auto value = po::value<std::vector<T>>();

  if (opt->default_value_supplied())
  {
    value->default_value(opt->default_value());
  }

  return add_notifier(opt, value)->composing();
}

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<T>>& opt)
{
  return get_base_boost_value(opt);
}

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::convert_to_boost_value(
    std::shared_ptr<typed_option<std::vector<T>>>& opt)
{
  return get_base_boost_value(opt)->multitoken();
}

template <>
po::typed_value<std::vector<bool>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<bool>>& opt);

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::add_notifier(
    std::shared_ptr<typed_option<T>>& opt, po::typed_value<std::vector<T>>* po_value)
{
  return po_value->notifier([opt](std::vector<T> final_arguments) {
    T first = final_arguments[0];
    for (auto const& item : final_arguments)
    {
      if (item != first)
      {
        std::stringstream ss;
        ss << "Disagreeing option values for '" << opt->m_name << "': '" << first << "' vs '" << item << "'";
        THROW_EX(VW::vw_argument_disagreement_exception, ss.str());
      }
    }

    // Set the value for the listening location.
    opt->m_location = first;
    opt->value(first);
  });
}

template <typename T>
po::typed_value<std::vector<T>>* options_boost_po::add_notifier(
    std::shared_ptr<typed_option<std::vector<T>>>& opt, po::typed_value<std::vector<T>>* po_value)
{
  return po_value->notifier([opt](std::vector<T> final_arguments) {
    // Set the value for the listening location.
    opt->m_location = final_arguments;
    opt->value(final_arguments);
  });
}

template <typename T>
bool options_boost_po::add_if_t(std::shared_ptr<base_option> opt, po::options_description& options_description)
{
  if (opt->m_type_hash == typeid(T).hash_code())
  {
    auto typed = std::dynamic_pointer_cast<typed_option<T>>(opt);
    add_to_description(typed, options_description);
    return true;
  }

  return false;
}

template <>
void options_boost_po::add_to_description_impl<typelist<>>(
    std::shared_ptr<base_option> opt, po::options_description& options_description);

template <typename T>
void options_boost_po::add_to_description(
    std::shared_ptr<typed_option<T>> opt, po::options_description& options_description)
{
  std::string boost_option_name = opt->m_name;
  if (opt->m_short_name != "")
  {
    boost_option_name += ",";
    boost_option_name += opt->m_short_name;
  }
  options_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(opt), opt->m_help.c_str());

  if (m_defined_options.count(opt->m_name) == 0)
  {
    // TODO may need to add noop notifier here.
    master_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(opt), "");
  }
}
}  // namespace config
}  // namespace VW
