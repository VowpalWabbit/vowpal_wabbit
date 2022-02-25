#include <iostream>
#include <exception>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <string>

#include "config/cli_help_formatter.h"
#include "config/option_builder.h"
#include "config/options_cli.h"
#include "config/option_group_definition.h"

#include "vw.h"
#include "parse_example_json.h"

enum class parser_type
{
  text,
  dsjson
};

parser_type to_parser_type(const std::string& str)
{
  if (str == "text") { return parser_type::text; }
  else if (str == "dsjson")
  {
    return parser_type::dsjson;
  }
  else
  {
    throw std::runtime_error("Unknown input type: " + str);
  }
}

int main(int argc, char** argv)
{
  VW::config::options_cli opts(std::vector<std::string>(argv + 1, argv + argc));

  bool help;
  std::string file_name;
  std::string extra_args;
  std::string type_str;
  VW::config::option_group_definition desc("Spanning Tree");
  desc.add(VW::config::make_option("help", help).short_name("h").help("Produce help message"))
      .add(VW::config::make_option("data", file_name).short_name("d").help("Data file to read. (required)"))
      .add(VW::config::make_option("args", extra_args).short_name("a").help("VW args to setup parser correctly"))
      .add(VW::config::make_option("type", type_str)
               .short_name("t")
               .help("Type of input format. [text, djson] (required)"));

  opts.add_and_parse(desc);
  // Return value is ignored as option reachability is not relevant here.
  opts.check_unregistered();

  // clang-format on

  if (help || (file_name.empty() && extra_args.empty() && type_str.empty()))
  {
    VW::config::cli_help_formatter help_formatter;
    std::cout << help_formatter.format_help(opts.get_all_option_group_definitions()) << std::endl;
    return 1;
  }

  if (type_str.empty())
  {
    std::cerr << "error: --type is required\n";
    return 1;
  }

  if (file_name.empty())
  {
    std::cerr << "error: --data is required\n";
    return 1;
  }

  std::string args = "--no_stdin --quiet ";
  if (opts.was_supplied("args"))
  {
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

  const auto type = to_parser_type(type_str);
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
    for (const auto& line : lines)
    {
      v_array<example*> examples;
      examples.push_back(&VW::get_unused_example(vw));
      VW::read_line_decision_service_json<false>(*vw, examples, const_cast<char*>(line.data()), line.length(), false,
          (VW::example_factory_t)&VW::get_unused_example, (void*)vw, &interaction);
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
