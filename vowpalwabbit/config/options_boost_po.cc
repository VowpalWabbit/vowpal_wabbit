// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "config/options_boost_po.h"
#include "io/logger.h"
#include "parse_primitives.h"
#include "io/logger.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <utility>

#include <boost/exception/exception.hpp>
#include <boost/throw_exception.hpp>

using namespace VW::config;

bool is_number(const VW::string_view& s)
{
  size_t endidx = 0;
  auto f = parseFloat(s.data(), endidx, s.data() + s.size());
  if ((endidx == 0 && !s.empty()) || std::isnan(f)) { return false; }

  return true;
}

options_boost_po::options_boost_po(int argc, char** argv)
    : options_boost_po(std::vector<std::string>(argv + 1, argv + argc))
{
}

options_boost_po::options_boost_po(const std::vector<std::string>& args) : m_command_line(args) {}

template <>
po::typed_value<std::vector<bool>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<bool>>& opt)
{
  auto value = get_base_boost_value(opt);

  if (opt->default_value_supplied())
  { THROW("Using a bool option type acts as a switch, no explicit default value is allowed.") }

  value->default_value({false}, "Default:false");
  value->zero_tokens();
  value->implicit_value({true}, "true");

  return add_notifier(opt, value);
}

void options_boost_po::add_to_description(
    const std::shared_ptr<base_option>& opt, po::options_description& options_description)
{
  add_to_description_impl<supported_options_types>(opt, options_description);
}

void options_boost_po::internal_add_and_parse(const option_group_definition& group)
{
  po::options_description new_options(group.m_name);
  for (const auto& opt_ptr : group.m_options)
  {
    add_to_description(opt_ptr, new_options);
    m_defined_options.insert(opt_ptr->m_name);
    m_defined_options.insert(opt_ptr->m_short_name);
    m_defined_options.insert("-" + opt_ptr->m_short_name);
  }

  try
  {
    po::variables_map vm;
    auto parsed_options = po::command_line_parser(m_command_line)
                              .options(new_options)
                              .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
                              .allow_unregistered()
                              .run();

    for (auto const& option : parsed_options.options)
    {
      // If the supplied option is interpreted as a number, then ignore it. There are no options like this and it is
      // just a false positive.
      if (is_number(option.string_key)) { m_ignore_supplied.insert(option.string_key); }

      m_supplied_options.insert(option.string_key);

      // If a std::string is later determined to be a value the erase it. This happens for negative numbers "-2"
      for (auto& val : option.value) { m_ignore_supplied.insert(val); }

      // Parsed options can contain short options in the form -k, we can only check these as the group definitions come
      // in.
      if (option.string_key.length() > 0 && option.string_key[0] == '-')
      {
        auto short_name = option.string_key.substr(1);
        for (const auto& opt_ptr : group.m_options)
        {
          if (opt_ptr->m_short_name == short_name) { m_supplied_options.insert(short_name); }
        }
      }
    }

    po::store(parsed_options, vm);
    po::notify(vm);
  }
// It seems as though boost::wrapexcept was introduced in 1.69 and it later started to be thrown out of Boost PO.
#if BOOST_VERSION >= 106900
  catch (boost::wrapexcept<boost::program_options::invalid_option_value>& ex)
  {
    THROW_EX(VW::vw_argument_invalid_value_exception, ex.what());
  }
#endif
  catch (boost::exception_detail::clone_impl<
      boost::exception_detail::error_info_injector<boost::program_options::invalid_option_value>>& ex)
  {
    THROW_EX(VW::vw_argument_invalid_value_exception, ex.what());
  }
  catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::bad_lexical_cast>>& ex)
  {
    THROW_EX(VW::vw_argument_invalid_value_exception, ex.what());
  }
  catch (boost::exception_detail::clone_impl<
      boost::exception_detail::error_info_injector<boost::program_options::ambiguous_option>>& ex)
  {
    THROW(ex.what());
  }
  catch (boost::program_options::ambiguous_option& ex)
  {
    THROW(ex.what());
  }

  const auto contains_necessary_options = group.contains_necessary_options();
  const auto is_necessary_enabled = group.check_necessary_enabled(*this);

  // These options are only reachable if necessary was also passed.
  for (const auto& opt_ptr : group.m_options)
  {
    if ((contains_necessary_options && is_necessary_enabled) || !contains_necessary_options)
    {
      m_reachable_options.insert(opt_ptr->m_name);
      m_reachable_options.insert(opt_ptr->m_short_name);
      m_reachable_options.insert("-" + opt_ptr->m_short_name);
    }

    if (contains_necessary_options)
    {
      // We need to convert the unordered set to an ordered one for stable output.
      std::set<std::string> necessary_flags_set(group.m_necessary_flags.begin(), group.m_necessary_flags.end());
      m_dependent_necessary_options[opt_ptr->m_name].push_back(necessary_flags_set);
      m_dependent_necessary_options[opt_ptr->m_short_name].push_back(necessary_flags_set);
      m_dependent_necessary_options["-" + opt_ptr->m_short_name].push_back(necessary_flags_set);
    }
  }
}

bool options_boost_po::was_supplied(const std::string& key) const
{
  // Best check, only valid after options parsed.
  if (m_supplied_options.count(key) > 0) { return true; }

  // Basic check, std::string match against command line.
  auto keys = {std::string("--" + key), std::string("-" + key)};
  return std::find_first_of(std::begin(m_command_line), std::end(m_command_line), std::begin(keys), std::end(keys)) !=
      std::end(m_command_line);
}

// Check all supplied arguments against defined args.
void options_boost_po::check_unregistered(VW::io::logger& logger)
{
  for (auto const& supplied : m_supplied_options)
  {
    if (m_defined_options.count(supplied) == 0 && m_ignore_supplied.count(supplied) == 0)
    { THROW_EX(VW::vw_unrecognised_option_exception, "unrecognised option '--" << supplied << "'"); }
  }

  for (auto const& supplied : m_supplied_options)
  {
    if (m_reachable_options.count(supplied) == 0 && m_ignore_supplied.count(supplied) == 0)
    {
      const auto& dependent_necessary_options = m_dependent_necessary_options.at(supplied);

      auto message = fmt::format(
          "Option '{}' depends on another option (or combination of options) which was not supplied. Possible "
          "combinations of options which would enable this option are:\n",
          supplied);
      for (const auto& group : dependent_necessary_options)
      { message += fmt::format("\t{}\n", fmt::join(group, ", ")); }

      logger.err_warn(message);
    }
  }
}

template <>
void options_boost_po::add_to_description_impl<typelist<>>(
    const std::shared_ptr<base_option>& opt, po::options_description& /*description*/)
{
  THROW(fmt::format("Option '{}' has an unsupported option type.", opt->m_name));
}

const std::set<std::string>& options_boost_po::get_supplied_options() const { return m_supplied_options; }

void options_boost_po::insert(const std::string& key, const std::string& value)
{
  m_command_line.push_back("--" + key);
  if (!value.empty()) { m_command_line.push_back(value); }
}

// Note: does not work for vector options.
void options_boost_po::replace(const std::string& key, const std::string& value)
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

std::vector<std::string> options_boost_po::get_positional_tokens() const
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
