#include "vw/config/cli_help_formatter.h"
#include "vw/config/option_builder.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options_cli.h"
#include "vw/core/io_buf.h"
#include "vw/core/learner.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/json_parser/parse_example_json.h"
#include "vw/text_parser/parse_example_text.h"

#ifdef VW_FEAT_CSV_ENABLED
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
  TEXT,
  DSJSON,
  CSV
};

parser_type to_parser_type(const std::string& str)
{
  if (str == "text") { return parser_type::TEXT; }
  else if (str == "dsjson") { return parser_type::DSJSON; }
  else if (str == "csv") { return parser_type::CSV; }
  else { throw std::runtime_error("Unknown input type: " + str); }
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
               .help("Type of input format. [text, dsjson, csv] (required)"));

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

  std::vector<std::string> args{"--no_stdin", "--quiet"};
  if (opts.was_supplied("args"))
  {
    const auto& illegal_options = {"--dsjson", "--json", "--data", "-d", "--csv"};
    for (const auto& illegal_option : illegal_options)
    {
      if (extra_args.find(illegal_option) != std::string::npos)
      {
        std::cerr << "error: Cannot pass '" << illegal_option << "' as an extra option.\n";
        return 1;
      }
    }
    auto split_args = VW::split_command_line(extra_args);
    args.insert(args.end(), split_args.begin(), split_args.end());
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
  else { std::cerr << "error: could not open file: '" << file_name << "'\n"; }

  // Other option is the parser can use this io_buf. It's using more memory for no good reason, unless we run out it
  // shouldnt matter in this test tool.
  VW::io_buf file_contents_as_io_buf;
  std::ifstream test_file(file_name, std::ios::binary);
  std::vector<char> file_contents((std::istreambuf_iterator<char>(test_file)), std::istreambuf_iterator<char>());
  file_contents_as_io_buf.add_file(VW::io::create_buffer_view(file_contents.data(), file_contents.size()));

  const auto type = to_parser_type(type_str);
  if (type == parser_type::DSJSON) { args.push_back("--dsjson"); }
  else if (type == parser_type::CSV)
  {
#ifndef VW_FEAT_CSV_ENABLED
    THROW("CSV parser not enabled. Please reconfigure cmake and rebuild with VW_FEAT_CSV=ON");
#endif

    args.push_back("--csv");
  }

  auto vw = VW::initialize(VW::make_unique<VW::config::options_cli>(args));
  const auto is_multiline = vw->l->is_multiline();

  const auto start = std::chrono::high_resolution_clock::now();
  if (type == parser_type::TEXT)
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

        auto* ae = &VW::get_unused_example(vw.get());
        VW::string_view example(line.c_str(), line.size());
        VW::parsers::text::details::substring_to_example(vw.get(), ae, example);
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
        VW::example& ae = VW::get_unused_example(vw.get());
        VW::string_view example(line.c_str(), line.size());
        VW::parsers::text::details::substring_to_example(vw.get(), &ae, example);
        VW::finish_example(*vw, ae);
      }
    }
  }
  else if (type == parser_type::DSJSON)
  {
    VW::parsers::json::decision_service_interaction interaction;
    for (const auto& line : file_contents_as_lines)
    {
      VW::multi_ex examples;
      examples.push_back(&VW::get_unused_example(vw.get()));
      VW::parsers::json::read_line_decision_service_json<false>(*vw, examples, const_cast<char*>(line.data()),
          line.length(), false, std::bind(VW::get_unused_example, vw.get()), &interaction);
      VW::finish_example(*vw, examples);
    }
  }
  else
  {
#ifdef VW_FEAT_CSV_ENABLED
    VW::multi_ex examples;
    examples.push_back(&VW::get_unused_example(vw.get()));
    while (VW::parsers::csv::parse_csv_examples(vw.get(), file_contents_as_io_buf, examples) != 0)
    {
      VW::finish_example(*vw, *examples[0]);
      examples.clear();
      examples.push_back(&VW::get_unused_example(vw.get()));
    }
    VW::finish_example(*vw, *examples[0]);
#else
    THROW("CSV parser not enabled. Please reconfigure cmake and rebuild with VW_FEAT_CSV=ON");
#endif
  }
  const auto end = std::chrono::high_resolution_clock::now();

  const auto time_in_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  const auto bytes_per_second = (bytes / static_cast<float>(time_in_microseconds)) * 1e6;
  const auto megabytes_per_second = bytes_per_second / 1e6;
  std::cout << bytes << " bytes parsed in " << time_in_microseconds << "μs" << std::endl;
  std::cout << megabytes_per_second << "MB/s" << std::endl;

  return 0;
}
