#pragma once

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <memory>
#include <vector>

#include "vw_exception.h"

#include "options.h"

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

struct options_boost_po : public arguments_i {
  options_boost_po(int argc, char** argv)
    : m_command_line(argv + 1, argv + argc)
  {}

  options_boost_po(std::vector<std::string> args)
    : m_command_line(args)
  {}

  template<typename T>
  bool mismatches_with_existing_option(typed_parameter<T>& param_to_check) {
    for (auto existing_param : m_existing_parameters) {
      if (existing_param->m_name == param_to_check.m_name) {
        if (existing_param->m_type_hash == param_to_check.m_type_hash) {
          auto cast_opt = std::dynamic_pointer_cast<typed_parameter<T>>(existing_param);
          if (*cast_opt != param_to_check) {
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
  typed_parameter<T>* get_equivalent_option_if_exists(typed_parameter<T>& param_to_check) {
    for (auto existing_param : m_existing_parameters) {
      if (existing_param->m_name == param_to_check.m_name) {
        if (existing_param->m_type_hash == param_to_check.m_type_hash) {
          auto cast_opt = std::dynamic_pointer_cast<typed_parameter<T>>(existing_param);
          if (*cast_opt == param_to_check) {
            return cast_opt.get();
          }
        }
      }
    }

    return nullptr;
  }

  template<typename T>
  bool add_if_t(std::shared_ptr<base_parameter> param, po::options_description& options_description) {
    if (param->m_type_hash == typeid(T).hash_code()) {
      auto typed = std::dynamic_pointer_cast<typed_parameter<T>>(param);
      add_to_description(typed, options_description);
      return true;
    }

    return false;
  }

  void add_to_description(std::shared_ptr<base_parameter> param, po::options_description& options_description) {
    if (add_if_t<int>(param, options_description)) { return; }
    if (add_if_t<float>(param, options_description)) { return; }
    if (add_if_t<char>(param, options_description)) { return; }
    if (add_if_t<std::string>(param, options_description)) { return; }
    if (add_if_t<bool>(param, options_description)) { return; }
    if (add_if_t<std::vector<int>>(param, options_description)) { return; }
    if (add_if_t<std::vector<float>>(param, options_description)) { return; }
    if (add_if_t<std::vector<char>>(param, options_description)) { return; }
    if (add_if_t<std::vector<std::string>>(param, options_description)) { return; }
    if (add_if_t<std::vector<bool>>(param, options_description)) { return; }

    throw std::invalid_argument("Unsupported type");
  }

  template<typename T>
  void add_to_description(std::shared_ptr<typed_parameter<T>> param, po::options_description& options_description) {
    if (mismatches_with_existing_option(*param)) {
      throw std::invalid_argument("Duplicate detected");
    }

    auto equiv = get_equivalent_option_if_exists(*param);
    if (equiv) {
      equiv->m_locations.insert(equiv->m_locations.end(), std::begin(param->m_locations), std::end(param->m_locations));
    }
    else {
      std::string boost_option_name = param->m_name + "," + param->m_short_name;
      options_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(param), param->m_help.c_str());
    }
  }

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_parameter<T>>& param) {
    return get_base_boost_value(param);
  }

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_parameter<std::vector<T>>>& param) {
    return get_base_boost_value(param)->multitoken();
  }

  template<>
  po::typed_value<std::vector<bool>>* convert_to_boost_value(std::shared_ptr<typed_parameter<bool>>& param) {
    return get_base_boost_value(param)->implicit_value({ true });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_parameter<T>>& param, po::typed_value<std::vector<T>>* po_value) {
    return po_value->notifier([this, param](std::vector<T> final_arguments) {
      T first = final_arguments[0];
      for (auto const& item : final_arguments) {
        if (item != first) {
          // TODO fix this error message
          throw po::validation_error(
            po::validation_error::invalid_option_value,
            param->m_name + "cannot take that value", "");
        }
      }

      // Set the value for all listening locations.
      for (auto location : param->m_locations) {
        *location = final_arguments[0];
      }

      if (param->m_keep) {
        // TODO keep
      }
    });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_parameter<std::vector<T>>>& param, po::typed_value<std::vector<T>>* po_value) {
    return po_value->notifier([this, param](std::vector<T> final_arguments) {
      // Set the value for all listening locations.
      for (auto location : param->m_locations) {
        *location = final_arguments;
      }

      if (param->m_keep) {
        // TODO keep
      }
    });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_parameter<T>>& param) {
    po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

    if (param->m_default_supplied) {
      value->default_value({ param->m_default_value });
    }

    return add_notifier(param, value)->composing();
  }

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_parameter<std::vector<T>>>& param) {
    po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

    if (param->m_default_supplied) {
      value->default_value(param->m_default_value);
    }

    return add_notifier(param, value)->composing();
  }

  virtual void add_and_parse(argument_group_definition group) override {
    po::options_description new_options(group.m_name);

    for (auto param_ptr : group.m_parameters) {
      add_to_description(param_ptr, new_options);
      m_existing_parameters.push_back(param_ptr);
    }

    merged_options.add(new_options);

    po::store(po::command_line_parser(m_command_line)
      .options(merged_options).allow_unregistered().run(), m_vm);

    po::notify(m_vm);
  }

  virtual bool was_supplied(std::string key) override {
    return m_vm.count(key) > 0;
  }

  virtual std::string help() override {
    return "";
  }

  virtual std::string get_kept() override {
    return "";
  }

private:
  std::vector<std::string> m_command_line;
  po::variables_map m_vm;
  po::options_description merged_options;

  std::vector<std::shared_ptr<base_parameter>> m_existing_parameters;
};
