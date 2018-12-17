#include "options_boost_po.h"

#include <sstream>

#include <algorithm>
#include <iterator>

using namespace VW::config;

template<>
po::typed_value<std::vector<bool>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<bool>>& opt) {
  return get_base_boost_value(opt)->default_value({ false })->implicit_value({ true });
}

void options_boost_po::add_to_description(std::shared_ptr<base_option> opt, po::options_description& options_description) {
  if (add_if_t<int>(opt, options_description)) { return; }
  if (add_if_t<size_t>(opt, options_description)) { return; }
  if (add_if_t<float>(opt, options_description)) { return; }
  if (add_if_t<double>(opt, options_description)) { return; }
  if (add_if_t<char>(opt, options_description)) { return; }
  if (add_if_t<std::string>(opt, options_description)) { return; }
  if (add_if_t<bool>(opt, options_description)) { return; }
  if (add_if_t<std::vector<int>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<size_t>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<float>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<double>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<char>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<std::string>>(opt, options_description)) { return; }

  THROW("That is an unsupported argument type.");
}

void options_boost_po::add_and_parse(option_group_definition group) {
  po::options_description new_options(group.m_name);

  // Add the help for the given options.
  new_options.print(m_help_stringstream);

  for (auto opt_ptr : group.m_options) {
    add_to_description(opt_ptr, new_options);
    m_defined_options.insert(opt_ptr->m_name);

    // Only the first object for a given key will be inserted
    m_options.insert(std::make_pair(opt_ptr->m_name, opt_ptr));
  }

  try {
    po::variables_map vm;
    auto parsed_options = po::command_line_parser(m_command_line)
      .options(new_options).allow_unregistered().run();

    for (auto const& option : parsed_options.options) {
      m_supplied_options.insert(option.string_key);
    }

    po::store(parsed_options, vm);
    po::notify(vm);
  }
  catch (boost::exception_detail::clone_impl<
    boost::exception_detail::error_info_injector<
    boost::program_options::invalid_option_value>>&ex) {
    THROW_EX(VW::vw_argument_invalid_value_exception, ex.what());
  }
  catch (boost::exception_detail::clone_impl<
    boost::exception_detail::error_info_injector<
    boost::program_options::ambiguous_option>>&ex) {
    THROW(ex.what());
  }
}

bool options_boost_po::was_supplied(std::string key) {
  return m_supplied_options.count(key) > 0;
}

std::string options_boost_po::help() {
  return m_help_stringstream.str();
}

std::vector<std::shared_ptr<base_option>> options_boost_po::get_all_options() {
  std::vector<std::shared_ptr<base_option>> output_values;

  std::transform(
    m_options.begin(),
    m_options.end(),
    std::back_inserter(output_values),
    [](auto &kv) { return kv.second; }
  );

  return output_values;
}

std::shared_ptr<base_option> VW::config::options_boost_po::get_option(std::string key) {
  auto it = m_options.find(key);
  if (it != m_options.end()) {
    return it->second;
  }

  throw std::out_of_range(key + " was not found.");
}

// Explicit run without allow_unregistered.
void options_boost_po::check_unregistered() {
  for (auto const& supplied : m_supplied_options) {
    if (m_defined_options.count(supplied) == 0) {
      THROW(supplied << " is not a recognized command line option");
    }
  }
}
