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

struct options_boost_po : public options_i {
  options_boost_po(int argc, char** argv)
    : m_arguments(argv + 1, argv + argc)
  {}

  options_boost_po(std::vector<std::string> args)
    : m_arguments(args)
  {}

  template<typename T>
  bool mismatches_with_existing_option(typed_option<T>& check_opt) {
    for (auto existing_opt : m_existing_options) {
      if (existing_opt->m_name == check_opt.m_name) {
        if (existing_opt->m_type_hash == check_opt.m_type_hash) {
          auto cast_opt = std::dynamic_pointer_cast<typed_option<T>>(existing_opt);
          if (*cast_opt != check_opt) {
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
  typed_option<T>* get_equivalent_option_if_exists(typed_option<T>& check_opt) {
    for (auto existing_opt : m_existing_options) {
      if (existing_opt->m_name == check_opt.m_name) {
        if (existing_opt->m_type_hash == check_opt.m_type_hash) {
          auto cast_opt = std::dynamic_pointer_cast<typed_option<T>>(existing_opt);
          if (*cast_opt == check_opt) {
            return cast_opt.get();
          }
        }
      }
    }

    return nullptr;
  }

  template<typename T>
  bool add_if_t(std::shared_ptr<base_option> option, po::options_description& options_description) {
    if (option->m_type_hash == typeid(T).hash_code()) {
      auto typed = std::dynamic_pointer_cast<typed_option<T>>(option);
      add_to_description(typed, options_description);
      return true;
    }

    return false;
  }

  void add_to_description(std::shared_ptr<base_option> option, po::options_description& options_description) {
    if (add_if_t<int>(option, options_description)) { return; }
    if (add_if_t<float>(option, options_description)) { return; }
    if (add_if_t<char>(option, options_description)) { return; }
    if (add_if_t<std::string>(option, options_description)) { return; }
    if (add_if_t<bool>(option, options_description)) { return; }
    if (add_if_t<std::vector<int>>(option, options_description)) { return; }
    if (add_if_t<std::vector<float>>(option, options_description)) { return; }
    if (add_if_t<std::vector<char>>(option, options_description)) { return; }
    if (add_if_t<std::vector<std::string>>(option, options_description)) { return; }
    if (add_if_t<std::vector<bool>>(option, options_description)) { return; }

    throw std::invalid_argument("Unsupported type");
  }

  template<typename T>
  void add_to_description(std::shared_ptr<typed_option<T>> option, po::options_description& options_description) {
    if (mismatches_with_existing_option(*option)) {
      throw std::invalid_argument("Duplicate detected");
    }

    auto equiv = get_equivalent_option_if_exists(*option);
    if (equiv) {
      equiv->m_locations.insert(equiv->m_locations.end(), std::begin(option->m_locations), std::end(option->m_locations));
    }
    else {
      std::string boost_option_name = option->m_name + "," + option->m_short_name;
      options_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(option), option->m_help.c_str());
    }
  }

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<T>>& option) {
    return get_base_boost_value(option);
  }

  template<typename T>
  po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& option) {
    return get_base_boost_value(option)->multitoken();
  }

  template<>
  po::typed_value<std::vector<bool>>* convert_to_boost_value(std::shared_ptr<typed_option<bool>>& option) {
    return get_base_boost_value(option)->implicit_value({ true });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_option<T>>& option, po::typed_value<std::vector<T>>* po_value) {
    return po_value->notifier([this, option](std::vector<T> args) {
      T first = args[0];
      for (auto const& item : args) {
        if (item != first) {
          // TODO fix this error message
          throw po::validation_error(
            po::validation_error::invalid_option_value,
            option->m_name + "cannot take that value", "");
        }
      }

      // Set the value for all listening locations.
      for (auto location : option->m_locations) {
        *location = args[0];
      }

      if (option->m_keep) {
        // TODO keep
      }
    });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_option<std::vector<T>>>& option, po::typed_value<std::vector<T>>* po_value) {
    return po_value->notifier([this, option](std::vector<T> arg) {
      // Set the value for all listening locations.
      for (auto location : option->m_locations) {
        *location = arg;
      }

      if (option->m_keep) {
        // TODO keep
      }
    });
  }

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<T>>& option) {
    po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

    if (option->m_default_supplied) {
      value->default_value({ option->m_default_value });
    }

    return add_notifier(option, value)->composing();
  }

  template<typename T>
  po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& option) {
    po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

    if (option->m_default_supplied) {
      value->default_value(option->m_default_value);
    }

    return add_notifier(option, value)->composing();
  }

  virtual void add_and_parse(option_group_definition group) override {
    po::options_description new_options(group.m_name);

    for (auto opt_ptr : group.m_options) {
      add_to_description(opt_ptr, new_options);
      m_existing_options.push_back(opt_ptr);
    }

    merged_options.add(new_options);

    po::store(po::command_line_parser(m_arguments)
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
  std::vector<std::string> m_arguments;
  po::variables_map m_vm;
  po::options_description merged_options;

  std::vector<std::shared_ptr<base_option>> m_existing_options;
};
