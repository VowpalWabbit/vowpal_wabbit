#include "vw/config/cli_help_formatter.h"
#include "vw/config/option_builder.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options_cli.h"
#include "vw/core/io_buf.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

#ifdef VW_BUILD_CSV
#  include "vw/csv_parser/parse_example_csv.h"
#endif

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

enum class parser_type
{
  text,
  dsjson,
  csv
};

parser_type to_parser_type(const std::string& str)
{
  if (str == "text") { return parser_type::text; }
  else if (str == "dsjson")
  {
    return parser_type::dsjson;
  }
  else if (str == "csv")
  {
    return parser_type::csv;
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
               .help("Type of input format. [text, djson, csv] (required)"));

  opts.add_and_parse(desc);
  // Return value is ignored as option reachability is not relevant here.
  auto result = opts.check_unregistered();
  _UNUSED(result);

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
    const auto& illegal_options = {"--djson", "--json", "--data", "-d", "--csv"};
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
  std::vector<std::string> file_contents_as_lines;
  std::ifstream file(file_name);
  if (file.is_open())
  {
    std::string line;
    while (std::getline(file, line))
    {
      bytes += line.size() * sizeof(std::string::value_type);
      file_contents_as_lines.push_back(std::move(line));
    }
    file.close();
  }
  else
  {
    std::cerr << "error: could not open file: '" << file_name << "'\n";
  }

  // Other option is the parser can use this io_buf. It's using more memory for no good reason, unless we run out it
  // shouldnt matter in this test tool.
  io_buf file_contents_as_io_buf;
  std::ifstream testFile(file_name, std::ios::binary);
  std::vector<char> fileContents((std::istreambuf_iterator<char>(testFile)), std::istreambuf_iterator<char>());
  file_contents_as_io_buf.add_file(VW::io::create_buffer_view(fileContents.data(), fileContents.size()));

  const auto type = to_parser_type(type_str);
  if (type == parser_type::dsjson) { args += " --dsjson"; }
  else if (type == parser_type::csv)
  {
#ifndef VW_BUILD_CSV
    THROW("CSV parser not enabled. Please reconfigure cmake and rebuild with VW_BUILD_CSV=ON");
#endif

    args += " --csv";
  }

  auto vw = VW::initialize(args, nullptr, false, nullptr, nullptr);
  const auto is_multiline = vw->l->is_multiline();

  const auto start = std::chrono::high_resolution_clock::now();
  if (type == parser_type::text)
  {
    if (is_multiline)
    {
      VW::multi_ex exs;
      for (const auto& line : file_contents_as_lines)
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
      for (const auto& line : file_contents_as_lines)
      {
        VW::example& ae = VW::get_unused_example(vw);
        VW::string_view example(line.c_str(), line.size());
        substring_to_example(vw, &ae, example);
        VW::finish_example(*vw, ae);
      }
    }
  }
  else if (type == parser_type::dsjson)
  {
    DecisionServiceInteraction interaction;
    for (const auto& line : file_contents_as_lines)
    {
      VW::multi_ex examples;
      examples.push_back(&VW::get_unused_example(vw));
      VW::read_line_decision_service_json<false>(*vw, examples, const_cast<char*>(line.data()), line.length(), false,
          (VW::example_factory_t)&VW::get_unused_example, (void*)vw, &interaction);
      VW::finish_example(*vw, examples);
    }
  }
  else
  {
#ifdef VW_BUILD_CSV
    VW::multi_ex examples;
    examples.push_back(&VW::get_unused_example(vw));
    while (VW::parsers::parse_csv_examples(vw, file_contents_as_io_buf, examples) != 0)
    {
      VW::finish_example(*vw, *examples[0]);
      examples.clear();
      examples.push_back(&VW::get_unused_example(vw));
    }
#else
    THROW("CSV parser not enabled. Please reconfigure cmake and rebuild with VW_BUILD_CSV=ON");
#endif
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
