// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "options_boost_po.h"
#include "io/logger.h"
#include "parse_primitives.h"
#include "io/logger.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <utility>
#include <regex>

#include <boost/exception/exception.hpp>
#include <boost/throw_exception.hpp>

using namespace VW::config;

std::ostream& std::operator<<(std::ostream& os, const std::vector<bool>& vec)
{
  // The lack of & is the only different bit to the template in the header.
  for (auto const item : vec) { os << item << ", "; }
  return os;
}

bool is_number(const VW::string_view& s)
{
  size_t endidx = 0;
  auto f = parseFloat(s.begin(), endidx, s.end());
  if ((endidx == 0 && !s.empty()) || std::isnan(f)) { return false; }

  return true;
}

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
  m_option_group_dic[m_current_reduction_tint].push_back(group);
  m_option_group_list.push_back(group);

  po::options_description new_options(group.m_name);

  for (const auto& opt_ptr : group.m_options)
  {
    add_to_description(opt_ptr, new_options);
    m_defined_options.insert(opt_ptr->m_name);
    m_defined_options.insert(opt_ptr->m_short_name);
    m_defined_options.insert("-" + opt_ptr->m_short_name);

    // The last definition is kept. There was a bug where using .insert at a later pointer changed the command line but
    // the previously defined option's default value was serialized into the model. This resolves that state info.
    m_options[opt_ptr->m_name] = opt_ptr;
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

std::vector<std::shared_ptr<base_option>> options_boost_po::get_all_options()
{
  std::vector<std::shared_ptr<base_option>> output_values;

  std::transform(m_options.begin(), m_options.end(), std::back_inserter(output_values),
      [](std::pair<const std::string, std::shared_ptr<base_option>>& kv) { return kv.second; });

  return output_values;
}

std::vector<std::shared_ptr<const base_option>> VW::config::options_boost_po::get_all_options() const
{
  std::vector<std::shared_ptr<const base_option>> output_values;
  output_values.reserve(m_options.size());
  for (const auto& kv : m_options) { output_values.push_back(kv.second); }
  return output_values;
}

// This function is called by both the const and non-const version. The const version will implicitly upgrade the
// shared_ptr to const
std::shared_ptr<base_option> internal_get_option(
    const std::string& key, const std::map<std::string, std::shared_ptr<VW::config::base_option>>& options)
{
  auto it = options.find(key);
  if (it != options.end()) { return it->second; }

  throw std::out_of_range(key + " was not found.");
}

std::shared_ptr<base_option> VW::config::options_boost_po::get_option(const std::string& key)
{
  return internal_get_option(key, m_options);
}

std::shared_ptr<const base_option> VW::config::options_boost_po::get_option(const std::string& key) const
{
  // shared_ptr can implicitly upgrade to const from non-const
  return internal_get_option(key, m_options);
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
