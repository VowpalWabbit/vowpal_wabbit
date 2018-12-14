#include "options_boost_po.h"

using namespace VW;

#include <sstream>

template<>
po::typed_value<std::vector<bool>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<bool>>& opt) {
  return get_base_boost_value(opt)->default_value({ false })->implicit_value({ true });
}

void options_boost_po::add_to_description(std::shared_ptr<base_option> opt, po::options_description& options_description) {
  if (add_if_t<int>(opt, options_description)) { return; }
  if (add_if_t<float>(opt, options_description)) { return; }
  if (add_if_t<char>(opt, options_description)) { return; }
  if (add_if_t<std::string>(opt, options_description)) { return; }
  if (add_if_t<bool>(opt, options_description)) { return; }
  if (add_if_t<std::vector<int>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<float>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<char>>(opt, options_description)) { return; }
  if (add_if_t<std::vector<std::string>>(opt, options_description)) { return; }

  THROW("That is an unsupported argument type.");
}

void options_boost_po::add_and_parse(option_group_definition group) {
  po::options_description new_options(group.m_name);

  for (auto param_ptr : group.m_options) {
    add_to_description(param_ptr, new_options);
  }

  m_merged_options.add(new_options);
  process_current_options_description();
}

bool options_boost_po::was_supplied(std::string key) {
  return m_supplied_options.count(key) > 0;
}

std::string options_boost_po::help() {
  std::stringstream ss;
  m_merged_options.print(ss);
  return ss.str();
}

void options_boost_po::merge(options_i* other) {
  auto all_other_args = other->get_all_options();

  for (auto param_ptr : all_other_args) {
    add_to_description(param_ptr, m_merged_options);
  }

  process_current_options_description();
}

std::vector<std::shared_ptr<base_option>>& options_boost_po::get_all_options() {
  return m_existing_options;
}

base_option& options_boost_po::get_option(std::string key) {
  for (auto const& arg : m_existing_options) {
    if (arg->m_name == key)
      return *arg;
  }

  throw std::out_of_range(key + " was not found.");
}

// Explicit run without allow_unregistered.
void options_boost_po::check_unregistered() {
  try {
    po::store(po::command_line_parser(m_command_line)
      .options(m_merged_options).run(), m_vm);
    po::notify(m_vm);
  }
  catch (boost::exception_detail::clone_impl<
    boost::exception_detail::error_info_injector<
    boost::program_options::unknown_option>>& ex) {
    THROW(ex.what());
  }
}

void options_boost_po::process_current_options_description()
{
  try {
    auto parsed_options = po::command_line_parser(m_command_line)
      .options(m_merged_options).allow_unregistered().run();

    for (auto const& option : parsed_options.options) {
      m_supplied_options.insert(option.string_key);
    }

    po::store(parsed_options, m_vm);
    po::notify(m_vm);
  }
  catch (boost::exception_detail::clone_impl<
    boost::exception_detail::error_info_injector<
    boost::program_options::invalid_option_value>>&ex) {
    THROW_EX(VW::vw_argument_invalid_value_exception, ex.what());
  }
}
