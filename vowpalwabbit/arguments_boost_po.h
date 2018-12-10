#pragma once

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <memory>
#include <vector>
#include <sstream>
#include <set>

#include "arguments.h"
#include "vw_exception.h"

// Boost Program Options requires that all types that have a default option are ostreamable
namespace std {
  template<typename T>
  std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    for (auto const& item : vec) {
      os << item << ", ";
    }
    return os;
  }
}

template std::ostream& std::operator<< <int>(std::ostream &, const std::vector<int>&);
template std::ostream& std::operator<< <char>(std::ostream &, const std::vector<char>&);
template std::ostream& std::operator<< <std::string> (std::ostream &, const std::vector<std::string>&);
template std::ostream& std::operator<< <float>(std::ostream &, const std::vector<float>&);
template std::ostream& std::operator<< <bool>(std::ostream &, const std::vector<bool>&);

namespace VW {

struct arguments_boost_po : public arguments_i {
  arguments_boost_po(int argc, char** argv)
    : m_command_line(argv + 1, argv + argc)
  {}

  arguments_boost_po(std::vector<std::string> args)
    : m_command_line(args)
  {}

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_argument<T>>& arg);

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_argument<std::vector<T>>>& arg);

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_argument<T>>& arg);

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_argument<std::vector<T>>>& arg);

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_argument<T>>& arg, po::typed_value<std::vector<T>>* po_value);

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_argument<std::vector<T>>>& arg, po::typed_value<std::vector<T>>* po_value);

  template<typename T>
  bool mismatches_with_existing_option(typed_argument<T>& arg_to_check);

  template<typename T>
  typed_argument<T>* get_equivalent_argument_if_exists(typed_argument<T>& arg_to_check);

  template<typename T>
  bool add_if_t(std::shared_ptr<base_argument> arg, po::options_description& options_description);

  void add_to_description(std::shared_ptr<base_argument> arg, po::options_description& options_description);

  template<typename T>
  void add_to_description(std::shared_ptr<typed_argument<T>> arg, po::options_description& options_description);

  virtual void add_and_parse(argument_group_definition group) override;
  virtual bool was_supplied(std::string key) override;
  virtual std::string help() override;
  virtual void check_unregistered() override;
  virtual void merge(arguments_i* other) override;
  virtual std::vector<std::shared_ptr<base_argument>>& get_all_args() override;
  virtual base_argument& get_arg(std::string key) override;

private:
  void process_current_options_description();

  std::vector<std::string> m_command_line;
  po::variables_map m_vm;
  po::options_description m_merged_options;
  std::set<std::string> m_supplied_options;
  std::string m_kept_command_line;

  std::vector<std::shared_ptr<base_argument>> m_existing_arguments;
};

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::get_base_boost_value(std::shared_ptr<typed_argument<T>>& arg) {
  po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

  if (arg->default_value_supplied()) {
    value->default_value({ arg->default_value() });
  }

  return add_notifier(arg, value)->composing();
}

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::get_base_boost_value(std::shared_ptr<typed_argument<std::vector<T>>>& arg) {
  po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

  if (arg->default_value_supplied()) {
    value->default_value(arg->default_value());
  }

  return add_notifier(arg, value)->composing();
}

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::convert_to_boost_value(std::shared_ptr<typed_argument<T>>& arg) {
  return get_base_boost_value(arg);
}

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::convert_to_boost_value(std::shared_ptr<typed_argument<std::vector<T>>>& arg) {
  return get_base_boost_value(arg)->multitoken();
}

template<>
po::typed_value<std::vector<bool>>* arguments_boost_po::convert_to_boost_value(std::shared_ptr<typed_argument<bool>>& arg);

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::add_notifier(std::shared_ptr<typed_argument<T>>& arg, po::typed_value<std::vector<T>>* po_value) {
  return po_value->notifier([this, arg](std::vector<T> final_arguments) {
    T first = final_arguments[0];
    for (auto const& item : final_arguments) {
      if (item != first) {
        std::stringstream ss;
        ss << "Disagreeing option values for '" << arg->m_name << "': '" << first << "' vs '" << item << "'";
        THROW_EX(VW::vw_argument_disagreement_exception, ss.str());
      }
    }

    // Set the value for all listening locations.
    for (auto location : arg->m_locations) {
      *location = first;
    }

    arg->value(first);
  });
}

template<typename T>
po::typed_value<std::vector<T>>* arguments_boost_po::add_notifier(std::shared_ptr<typed_argument<std::vector<T>>>& arg, po::typed_value<std::vector<T>>* po_value) {
  return po_value->notifier([this, arg](std::vector<T> final_arguments) {
    // Set the value for all listening locations.
    for (auto location : arg->m_locations) {
      *location = final_arguments;
    }

    arg->value(final_arguments);
  });
}

template<typename T>
bool arguments_boost_po::mismatches_with_existing_option(typed_argument<T>& arg_to_check) {
  for (auto existing_arg : m_existing_arguments) {
    if (existing_arg->m_name == arg_to_check.m_name) {
      if (existing_arg->m_type_hash == arg_to_check.m_type_hash) {
        auto cast_arg = std::dynamic_pointer_cast<typed_argument<T>>(existing_arg);
        if (*cast_arg != arg_to_check) {
          return true;
        }
      }
      else {
        return true;
      }
    }
  }

  return false;
}

template<typename T>
typed_argument<T>* arguments_boost_po::get_equivalent_argument_if_exists(typed_argument<T>& arg_to_check) {
  for (auto existing_arg : m_existing_arguments) {
    if (existing_arg->m_name == arg_to_check.m_name) {
      if (existing_arg->m_type_hash == arg_to_check.m_type_hash) {
        auto cast_arg = std::dynamic_pointer_cast<typed_argument<T>>(existing_arg);
        if (*cast_arg == arg_to_check) {
          return cast_arg.get();
        }
      }
    }
  }

  return nullptr;
}

template<typename T>
bool arguments_boost_po::add_if_t(std::shared_ptr<base_argument> arg, po::options_description& options_description) {
  if (arg->m_type_hash == typeid(T).hash_code()) {
    auto typed = std::dynamic_pointer_cast<typed_argument<T>>(arg);
    add_to_description(typed, options_description);
    return true;
  }

  return false;
}

template<typename T>
void arguments_boost_po::add_to_description(std::shared_ptr<typed_argument<T>> arg, po::options_description& options_description) {
  if (mismatches_with_existing_option(*arg)) {
    THROW("There already exists an option with name '" << arg->m_name << "' with a different spec. To subscribe for this value, the spec must match.");
  }

  auto equiv = get_equivalent_argument_if_exists(*arg);
  if (equiv) {
    equiv->m_locations.insert(equiv->m_locations.end(), std::begin(arg->m_locations), std::end(arg->m_locations));
  }
  else {
    std::string boost_option_name = arg->m_name;
    if (arg->m_short_name != "") {
      boost_option_name += ",";
      boost_option_name += arg->m_short_name;
    }
    options_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(arg), arg->m_help.c_str());
    m_existing_arguments.push_back(arg);
  }
}
}
