#pragma once

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <memory>
#include <vector>
#include <sstream>
#include <set>

#include "options.h"
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
  namespace config {
    struct options_boost_po : public options_i {
      options_boost_po(int argc, char** argv) : options_boost_po(std::vector<std::string>(argv + 1, argv + argc))
      {}

      options_boost_po(std::vector<std::string> args)
        : m_command_line(args)
      {
        //// Be friendly: if -d was left out, treat positional param as data file
        //po::positional_options_description pos_description;
        //po::options_description opt_description;
        //opt_description.add_options()("data", po::value<std::string>());

        //pos_description.add("data", -1);
        //po::parsed_options parsed = po::command_line_parser(args).options(opt_description).positional(pos_description).allow_unregistered().run();

        //po::variables_map vm;
        //po::store(parsed, vm);
        //
        //if (vm.count("data") > 0) {
        //  args.push_back("--data");
        //  args.push_back(vm["data"].as<std::string>());
        //}
      }

      virtual void add_and_parse(option_group_definition group) override;
      virtual bool was_supplied(std::string key) override;
      virtual std::string help() override;
      virtual void check_unregistered() override;
      virtual std::vector<std::shared_ptr<base_option>> get_all_options() override;
      virtual std::shared_ptr<base_option> get_option(std::string key) override;

      virtual void insert(std::string key, std::string value) override {
        m_command_line.push_back("--" + key);
        m_command_line.push_back(value);
      }

    private:
      template<typename T>
      po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<T>>& opt);

      template<typename T>
      po::typed_value<std::vector<T>>* get_base_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt);

      template<typename T>
      po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<T>>& opt);

      template<typename T>
      po::typed_value<std::vector<T>>* convert_to_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt);

      template<typename T>
      po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_option<T>>& opt, po::typed_value<std::vector<T>>* po_value);

      template<typename T>
      po::typed_value<std::vector<T>>* add_notifier(std::shared_ptr<typed_option<std::vector<T>>>& opt, po::typed_value<std::vector<T>>* po_value);

      template<typename T>
      bool add_if_t(std::shared_ptr<base_option> opt, po::options_description& options_description);

      void add_to_description(std::shared_ptr<base_option> opt, po::options_description& options_description);

      template<typename T>
      void add_to_description(std::shared_ptr<typed_option<T>> opt, po::options_description& options_description);

    private:
      std::map<std::string, std::shared_ptr<base_option>> m_options;

      std::vector<std::string> m_command_line;

      std::stringstream m_help_stringstream;

      // All options that were supplied on the command line.
      std::set<std::string> m_supplied_options;

      // All options that a description was provided for.
      std::set<std::string> m_defined_options;
    };

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::get_base_boost_value(std::shared_ptr<typed_option<T>>& opt) {
      po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

      if (opt->default_value_supplied()) {
        value->default_value({ opt->default_value() });
      }

      return add_notifier(opt, value)->composing();
    }

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::get_base_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt) {
      po::typed_value<std::vector<T>>* value = po::value<std::vector<T>>();

      if (opt->default_value_supplied()) {
        value->default_value(opt->default_value());
      }

      return add_notifier(opt, value)->composing();
    }

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<T>>& opt) {
      return get_base_boost_value(opt);
    }

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<std::vector<T>>>& opt) {
      return get_base_boost_value(opt)->multitoken();
    }

    template<>
    po::typed_value<std::vector<bool>>* options_boost_po::convert_to_boost_value(std::shared_ptr<typed_option<bool>>& opt);

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::add_notifier(std::shared_ptr<typed_option<T>>& opt, po::typed_value<std::vector<T>>* po_value) {
      return po_value->notifier([this, opt](std::vector<T> final_arguments) {
        T first = final_arguments[0];
        for (auto const& item : final_arguments) {
          if (item != first) {
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

    template<typename T>
    po::typed_value<std::vector<T>>* options_boost_po::add_notifier(std::shared_ptr<typed_option<std::vector<T>>>& opt, po::typed_value<std::vector<T>>* po_value) {
      return po_value->notifier([this, opt](std::vector<T> final_arguments) {
        // Set the value for the listening location.
        opt->m_location = final_arguments;
        opt->value(final_arguments);
      });
    }

    template<typename T>
    bool options_boost_po::add_if_t(std::shared_ptr<base_option> opt, po::options_description& options_description) {
      if (opt->m_type_hash == typeid(T).hash_code()) {
        auto typed = std::dynamic_pointer_cast<typed_option<T>>(opt);
        add_to_description(typed, options_description);
        return true;
      }

      return false;
    }

    template<typename T>
    void options_boost_po::add_to_description(std::shared_ptr<typed_option<T>> opt, po::options_description& options_description) {
      std::string boost_option_name = opt->m_name;
      if (opt->m_short_name != "") {
        boost_option_name += ",";
        boost_option_name += opt->m_short_name;
      }
      options_description.add_options()(boost_option_name.c_str(), convert_to_boost_value(opt), opt->m_help.c_str());
    }
  }
}
