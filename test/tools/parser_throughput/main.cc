#include <iostream>
#include <exception>
#include <fstream>
#include <chrono>

#include <boost/program_options.hpp>

#include "vw.h"
#include "parse_example_json.h"

namespace po = boost::program_options;

enum class parser_type
{
  text,
  dsjson
};

void validate(boost::any& v, std::vector<std::string> const& values, parser_type* /*target_type*/, int)
{
  // Make sure no previous assignment to 'v' was made.
  po::validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  const auto& str = po::validators::get_single_string(values);

  if (str == "text")
  {
    v = boost::any(parser_type::text);
  }
  else if (str == "dsjson")
  {
    v = boost::any(parser_type::dsjson);
  }
  else
  {
    throw po::validation_error(po::validation_error::invalid_option_value);
  }
}

int main(int argc, char** argv)
{
  // clang-format off
  po::options_description desc("Parser throughput tool - determine throughput of VW parser");
  desc.add_options()
    ("help,h", "Produce help message")
    ("data,d", po::value<std::string>(), "Data file to read")
    ("args,a", po::value<std::string>(), "VW args to setup parser correctly")
    ("type,t", po::value<parser_type>(), "Type of input format. [text, djson]");
  // clang-format on
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch (...)
  {
    std::cerr << "Exception of unknown type!\n";
  }

  if (vm.count("help") || vm.size() == 0)
  {
    std::cout << desc << "\n";
    return 1;
  }

  if (vm.count("type") == 0)
  {
    std::cerr << "error: --type is required\n";
    return 1;
  }

  if (vm.count("data") == 0)
  {
    std::cerr << "error: --data is required\n";
    return 1;
  }

  std::string args = "--no_stdin --quiet ";
  if (vm.count("args") != 0)
  {
    const auto& extra_args = vm["args"].as<std::string>();
    const auto& illegal_options = {"--djson", "--json", "--data", "-d"};
    for (const auto& illegal_option : illegal_options)
    {
      if (extra_args.find(illegal_option) != std::string::npos)
      {
        std::cerr << "error: Cannot pass '" << illegal_option << "' as an extra option.\n";
        return 1;
      }
    }
    args += extra_args;
  }

  size_t bytes = 0;
  std::vector<std::string> lines;
  const auto file_name = vm["data"].as<std::string>();
  std::ifstream file(file_name);
  if (file.is_open())
  {
    std::string line;
    while (std::getline(file, line))
    {
      bytes += line.size() * sizeof(std::string::value_type);
      lines.push_back(std::move(line));
    }
    file.close();
  }
  else
  {
    std::cerr << "error: could not open file: '" << file_name << "'\n";
  }

  const auto type = vm["type"].as<parser_type>();
  if (type == parser_type::dsjson)
  {
    args += " --dsjson";
  }

  auto vw = VW::initialize(args, nullptr, false, nullptr, nullptr);
  const auto is_multiline = vw->l->is_multiline();

  const auto start = std::chrono::high_resolution_clock::now();
  if (type == parser_type::text)
  {
    if (is_multiline)
    {
      multi_ex exs;
      for (const auto& line : lines)
      {
        if (line.empty() && !exs.empty())
        {
          VW::finish_example(*vw, exs);
          exs.clear();
        }

        auto* ae = &VW::get_unused_example(vw);
        VW::string_view example(line.c_str(), line.size());
        substring_to_example(vw, ae, example);
        exs.push_back(ae);
      }

      if (!exs.empty())
      {
        VW::finish_example(*vw, exs);
        exs.clear();
      }
    }
    else
    {
      for (const auto& line : lines)
      {
        example& ae = VW::get_unused_example(vw);
        VW::string_view example(line.c_str(), line.size());
        substring_to_example(vw, &ae, example);
        VW::finish_example(*vw, ae);
      }
    }
  }
  else
  {
    DecisionServiceInteraction interaction;
    auto dsjson_parser = VW::make_dsjson_parser(*vw, false, true, false);
    for (const auto& line : lines)
    {
      v_array<example*> examples;
      examples.push_back(&VW::get_unused_example(vw));
      dsjson_parser->parse_object(const_cast<char*>(line.data()), line.length(),examples, interaction);
      multi_ex result;
      result.reserve(examples.size());
      for (size_t i = 0; i < examples.size(); ++i)
      {
        result.push_back(examples[i]);
      }
      // TODO - finish_example should support a v_array as input.
      VW::finish_example(*vw, result);
    }
  }
  const auto end = std::chrono::high_resolution_clock::now();

  const auto time_in_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  const auto bytes_per_second = (bytes / static_cast<float>(time_in_microseconds)) * 1e6;
  const auto megabytes_per_second = bytes_per_second / 1e6;
  std::cout << bytes << " bytes parsed in " << time_in_microseconds << "μs" << std::endl;
  std::cout << megabytes_per_second << "MB/s" << std::endl;

  VW::finish(*vw);

  return 0;
}
