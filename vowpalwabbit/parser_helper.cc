#include <boost/foreach.hpp>
#include "parser_helper.h"
#include <iostream>
#include <set>

using namespace std;

std::vector<std::string> opts_to_args(const std::vector<boost::program_options::option>& opts)
{
  std::vector<std::string> args;

  BOOST_FOREACH(const boost::program_options::option& option, opts)
  {
    if (option.unregistered)
    {
      args.insert(args.end(), option.original_tokens.begin(), option.original_tokens.end());
      continue;
    }

    if (option.value.empty())
    {
      args.push_back("--" + option.string_key);
      continue;
    }

    BOOST_FOREACH(const std::string& value, option.value)
    {
      if (option.string_key.length() > 0)
        args.push_back("--" + option.string_key);
      args.push_back(value);
    }
  }

  return args;
}

// blackbox wrapping of boost program options to ignore duplicate specification of options allowed only ones, but specified multiple times
// Behavior: only the first occurence is kept
// Strategy: add one argument after each other until we trigger multiple_occurrences exception. Special care has to be taken of arguments to options.
po::variables_map arguments::add_options_skip_duplicates(po::options_description& opts, bool do_notify)
{
  std::vector<std::string> dup_args(args);
  po::variables_map new_vm;

  for (int i = 0; i<2; i++)
    {
      // i = 0: initial parse attempt
      // i = 1: retry attempt after removing dups
      try
        {
          po::parsed_options parsed = po::command_line_parser(dup_args).
            style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
            options(opts).allow_unregistered().run();
          po::store(parsed, new_vm);

          // unique multi elements to avoid infinite growth
          for (auto& it : new_vm)
            {
              if (it.second.value().type() == typeid(vector<string>))
                {
                  auto& values = it.second.as<vector<string>>();
                  set<string> unique_set;
                  auto current_head = values.begin();
                  for (auto current_check = values.begin(); current_check != values.end(); current_check++)
                    if (unique_set.find(*current_check) == unique_set.end())
                      {
                        unique_set.insert(*current_check);
                        *current_head = *current_check;
                        current_head++;
                      }
                  values.erase(current_head, values.end());
                }
            }

          if (do_notify)
            po::notify(new_vm);

          // re-create args after unique
          args = opts_to_args(parsed.options);
          return new_vm;
        }
      catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::multiple_occurrences>>&)
        { }

      cout << "args = ";
      for(auto i: args)
        cout << " " << i;
      cout << endl;
      dup_args.clear();
      bool previous_option_needs_argument = false;
      for (auto&& arg : args)
        {
          new_vm.clear();
          dup_args.push_back(arg);
          try
            {
              po::parsed_options parsed = po::command_line_parser(dup_args).
                style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
                options(opts).allow_unregistered().run();
              po::store(parsed, new_vm);

              previous_option_needs_argument = false;
            }
          catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::multiple_occurrences>>& multi_occ)
            {
              auto ignored = arg;

              dup_args.pop_back();
              if (previous_option_needs_argument)
                {
                  auto option = dup_args.back();
                  auto duplicate_value = ignored;
                  ignored =  option + " " + ignored;
                  dup_args.pop_back();

                  // check if at least the values are the same

                  // reparse arguments so far
                  new_vm.clear();
                  po::parsed_options parsed_full = po::command_line_parser(dup_args).
                    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
                    options(opts).allow_unregistered().run();
                  po::store(parsed_full, new_vm);

                  // parse just the duplicate option
                  vector<string> sub_args;
                  sub_args.push_back(option);
                  sub_args.push_back(duplicate_value);

                  po::variables_map sub_vm;
                  po::parsed_options parsed_dup = po::command_line_parser(sub_args).
                    style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
                    options(opts).allow_unregistered().run();
                  po::store(parsed_dup, sub_vm);

                  // we need to compare the parsed actions to overcome different representation of the same value
                  // e.g. --epsilon 0.1 vs --epsilon 0.10000
                  string opt_name = multi_occ.get_option_name().substr(2);
                  auto duplicate_option = sub_vm.find(opt_name);
                  auto first_option_occurrence = new_vm.find(opt_name);

                  if (first_option_occurrence == new_vm.end() || duplicate_option == sub_vm.end())
                    THROW("unable to find duplicate option");

                  bool found_disagreement = false;
                  if (duplicate_option->second.value().type() == typeid(string))
                    found_disagreement = duplicate_option->second.as<string>() != first_option_occurrence->second.as<string>();
                  else if (duplicate_option->second.value().type() == typeid(float))
                    found_disagreement = duplicate_option->second.as<float>() != first_option_occurrence->second.as<float>();
                  else if (duplicate_option->second.value().type() == typeid(double))
                    found_disagreement = duplicate_option->second.as<double>() != first_option_occurrence->second.as<double>();
                  else if (duplicate_option->second.value().type() == typeid(int))
                    found_disagreement = duplicate_option->second.as<int>() != first_option_occurrence->second.as<int>();
                  else if (duplicate_option->second.value().type() == typeid(size_t))
                    found_disagreement = duplicate_option->second.as<size_t>() != first_option_occurrence->second.as<size_t>();
                  else if (duplicate_option->second.value().type() == typeid(uint32_t))
                    found_disagreement = duplicate_option->second.as<uint32_t>() != first_option_occurrence->second.as<uint32_t>();
                  else if (duplicate_option->second.value().type() == typeid(bool))
                    found_disagreement = duplicate_option->second.as<bool>() != first_option_occurrence->second.as<bool>();
                  else
                    THROW("Unsupported type for option '" << duplicate_option->first << "'");

                  if (found_disagreement)
                    {
                      // get the original string value
                      auto duplicate_option = parsed_dup.options.begin();
                      auto first_option_occurrence = std::find_if(parsed_full.options.begin(), parsed_full.options.end(),
                                                                  [&duplicate_option](po::option& o) { return duplicate_option->string_key == o.string_key; });

                      if (first_option_occurrence == parsed_full.options.end() || duplicate_option == parsed_dup.options.end())
                        THROW("unable to find duplicate option");

                      auto duplicate_option_value = *duplicate_option->value.begin();
                      auto first_option_occurrence_value = *first_option_occurrence->value.begin();

                      THROW_EX(VW::vw_argument_disagreement_exception, "Disagreeing option values for '" << option << "': '" << first_option_occurrence_value << "' vs '" << duplicate_option_value << "'");
                    }
                  }

              trace_message << "ignoring duplicate option: '" << ignored << "'" << endl;
              }
          catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_command_line_syntax>>& e)
            {
              // remember that this option needs an argument to be able to remove the option along with the argument
              // in the next iteration
              if (e.kind() == e.missing_parameter)
                previous_option_needs_argument = true;
            }
          catch (...)
            {
              // ignore anything else
            }
        }

      new_vm.clear();
      // parse ones more to trigger any other exception
    }

  THROW("failed to de-duplicate arguments");
}
