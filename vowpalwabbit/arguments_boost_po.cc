#include "arguments_boost_po.h"

using namespace VW;

#include <sstream>

template<>
po::typed_value<std::vector<bool>>* arguments_boost_po::convert_to_boost_value(std::shared_ptr<typed_argument<bool>>& arg) {
  return get_base_boost_value(arg)->default_value({ false })->implicit_value({ true });
}

void arguments_boost_po::add_to_description(std::shared_ptr<base_argument> arg, po::options_description& options_description) {
  if (add_if_t<int>(arg, options_description)) { return; }
  if (add_if_t<float>(arg, options_description)) { return; }
  if (add_if_t<char>(arg, options_description)) { return; }
  if (add_if_t<std::string>(arg, options_description)) { return; }
  if (add_if_t<bool>(arg, options_description)) { return; }
  if (add_if_t<std::vector<int>>(arg, options_description)) { return; }
  if (add_if_t<std::vector<float>>(arg, options_description)) { return; }
  if (add_if_t<std::vector<char>>(arg, options_description)) { return; }
  if (add_if_t<std::vector<std::string>>(arg, options_description)) { return; }

  THROW("That is an unsupported argument type.");
}

void arguments_boost_po::add_and_parse(argument_group_definition group) {
  po::options_description new_options(group.m_name);

  for (auto param_ptr : group.m_arguments) {
    add_to_description(param_ptr, new_options);
  }

  m_merged_options.add(new_options);
  process_current_options_description();
}

bool arguments_boost_po::was_supplied(std::string key) {
  return m_supplied_options.count(key) > 0;
}

std::string arguments_boost_po::help() {
  std::stringstream ss;
  m_merged_options.print(ss);
  return ss.str();
}

void arguments_boost_po::merge(arguments_i* other) {
  auto all_other_args = other->get_all_args();

  for (auto param_ptr : all_other_args) {
    add_to_description(param_ptr, m_merged_options);
  }

  process_current_options_description();
}

std::vector<std::shared_ptr<base_argument>>& arguments_boost_po::get_all_args() {
  return m_existing_arguments;
}

base_argument& arguments_boost_po::get_arg(std::string key) {
  for (auto const& arg : m_existing_arguments) {
    if (arg->m_name == key)
      return *arg;
  }

  throw std::out_of_range(key + " was not found.");
}

// Explicit run without allow_unregistered.
void arguments_boost_po::check_unregistered() {
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

void arguments_boost_po::process_current_options_description()
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
