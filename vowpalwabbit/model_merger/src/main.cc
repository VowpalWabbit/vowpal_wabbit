// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/vw_exception.h"
#include "vw/config/cli_help_formatter.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/memory.h"
#include "vw/core/merge.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"

#include <fstream>
#include <vector>

using namespace VW::config;

void print_help(const options_cli& options)
{
  const auto& option_groups = options.get_all_option_group_definitions();

  VW::config::cli_help_formatter formatter;
  std::cout << R"(Usage: vw-merge [options] <model1> <model2> ... <modelN>

    Merges multiple VW models into a single model. Models must be compatible.

    Note: This is an experimental tool.
)" << std::endl;
  std::cout << formatter.format_help(option_groups);
}

class logger_context
{
public:
  VW::io::logger& logger;
  std::string model_file_name;
};

void logger_output_func(void* void_context, VW::io::log_level level, const std::string& message)
{
  auto* context = static_cast<logger_context*>(void_context);
  auto newline_stripped_message = message;
  newline_stripped_message.erase(std::remove(newline_stripped_message.begin(), newline_stripped_message.end(), '\n'),
      newline_stripped_message.end());
  switch (level)
  {
    case VW::io::log_level::INFO_LEVEL:
      context->logger.info("({}): {}", context->model_file_name, newline_stripped_message);
      break;
    case VW::io::log_level::WARN_LEVEL:
      context->logger.warn("({}): {}", context->model_file_name, newline_stripped_message);
      break;
    case VW::io::log_level::ERROR_LEVEL:
      context->logger.error("({}): {}", context->model_file_name, newline_stripped_message);
      break;
    case VW::io::log_level::CRITICAL_LEVEL:
      context->logger.critical("({}): {}", context->model_file_name, newline_stripped_message);
      break;
    case VW::io::log_level::OFF_LEVEL:
      break;
    default:
      THROW("Unsupported log level");
  }
}

class command_line_options
{
public:
  VW::io::log_level log_level{};
  VW::io::output_location log_output_stream{};
  std::string output_file;
  std::string base_file;
  std::vector<std::string> input_files;
};

command_line_options parse_command_line(int argc, char** argv, VW::io::logger& logger)
{
  std::string log_level;
  std::string log_output_stream;
  bool help = false;
  option_group_definition diagnostics_options("Diagnostics");
  diagnostics_options.add(make_option("log_level", log_level)
                              .default_value("info")
                              .one_of({"info", "warn", "error", "critical", "off"})
                              .help("Log level for logging messages."));
  diagnostics_options.add(make_option("log_output", log_output_stream)
                              .default_value("stderr")
                              .one_of({"stdout", "stderr"})
                              .help("Specify the stream to output log messages to."));
  diagnostics_options.add(make_option("help", help).short_name("h").help("Output this help message."));

  std::string output_file;
  std::string base_file;
  option_group_definition output_options("Merge models");
  output_options.add(
      make_option("output", output_file).short_name('o').help("Name of file of merged model. Required."));
  output_options.add(make_option("base", base_file).short_name('b').help("Name of file the base model."));

  std::vector<std::string> args(argv + 1, argv + argc);
  options_cli options(args);

  options.add_and_parse(diagnostics_options);
  options.add_and_parse(output_options);
  auto warnings = options.check_unregistered();
  _UNUSED(warnings);

  if (help)
  {
    print_help(options);
    std::exit(0);
  }

  const auto model_files = options.get_positional_tokens();
  if (model_files.size() < 2)
  {
    logger.error("Must specify at least two model files to merge.");
    print_help(options);
    std::exit(1);
  }

  if (!options.was_supplied("output"))
  {
    logger.error("Must specify an output file.");
    print_help(options);
    std::exit(1);
  }

  command_line_options result;
  result.log_level = VW::io::get_log_level(log_level);
  result.log_output_stream = VW::io::get_output_location(log_output_stream);
  result.output_file = output_file;
  result.base_file = base_file;
  result.input_files = model_files;

  return result;
}

int main(int argc, char* argv[])
{
  auto logger = VW::io::create_default_logger();
  try
  {
    auto options = parse_command_line(argc, argv, logger);
    logger.set_level(options.log_level);
    logger.set_location(options.log_output_stream);

    std::vector<std::unique_ptr<VW::workspace>> models;
    std::vector<logger_context> logger_contexts;
    for (const auto& model_file : options.input_files)
    {
      logger.info("Loading model: {}", model_file);
      logger_contexts.push_back(logger_context{logger, model_file});

      auto custom_logger = VW::io::create_custom_sink_logger(&logger_contexts.back(), logger_output_func);
      auto model = VW::initialize(VW::make_unique<VW::config::options_cli>(std::vector<std::string>{
                                      "--driver_output_off", "--preserve_performance_counters"}),
          VW::io::open_file_reader(model_file), nullptr, nullptr, &custom_logger);
      models.push_back(std::move(model));
    }

    logger_contexts.push_back(logger_context{logger, "dest: " + options.input_files[0]});
    auto custom_logger = VW::io::create_custom_sink_logger(&logger_contexts.back(), logger_output_func);
    std::vector<const VW::workspace*> const_workspaces;
    const_workspaces.reserve(models.size());
    for (const auto& model : models) { const_workspaces.push_back(model.get()); }

    std::unique_ptr<VW::workspace> base_model = nullptr;
    if (!options.base_file.empty())
    {
      logger.info("Loading base model: {}", options.base_file);
      logger_contexts.push_back(logger_context{logger, "base: " + options.base_file});
      auto custom_logger = VW::io::create_custom_sink_logger(&logger_contexts.back(), logger_output_func);
      base_model = VW::initialize(VW::make_unique<VW::config::options_cli>(std::vector<std::string>{
                                      "--driver_output_off", "--preserve_performance_counters"}),
          VW::io::open_file_reader(options.base_file), nullptr, nullptr, &custom_logger);
    }

    auto merged = VW::merge_models(base_model.get(), const_workspaces, &custom_logger);

    logger.info("Saving model: {}", options.output_file);
    VW::save_predictor(*merged, options.output_file);
  }
  catch (const VW::vw_exception& e)
  {
    logger.critical("({}:{}): {}", e.filename(), e.line_number(), e.what());
    return 1;
  }
  catch (const std::exception& e)
  {
    logger.critical("{}", e.what());
    return 1;
  }

  return 0;
}
