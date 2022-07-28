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
#include "vw/core/parse_regressor.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"
#include "vw/io/logger.h"

#include <fstream>

using namespace VW::config;

void print_help(const options_cli& options)
{
  const auto& option_groups = options.get_all_option_group_definitions();

  VW::config::cli_help_formatter formatter;
  std::cout << R"(Usage: vw-merge [options] <model1> <model2> ... <modelN>

    Merges multiple VW models into a single model. Models must be compatible.
)" << std::endl;
  std::cout << formatter.format_help(option_groups);
}

int main(int argc, char* argv[])
{
  auto logger = VW::io::create_default_logger();
  try
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
    option_group_definition output_options("Ouput");
    output_options.add(
        make_option("output", output_file).short_name('o').help("Name of file of merged model. Required."));

    std::vector<std::string> args(argv + 1, argv + argc);
    options_cli options(args);

    options.add_and_parse(diagnostics_options);
    options.add_and_parse(output_options);
    options.check_unregistered();

    if (help)
    {
      print_help(options);
      std::exit(0);
    }

    logger.set_level(VW::io::get_log_level(log_level));
    logger.set_location(VW::io::get_output_location(log_output_stream));

    const auto model_files = options.get_positional_tokens();

    if (model_files.size() < 2)
    {
      logger.error("Must specify at least two model files to merge.");
      print_help(options);
      std::exit(1);
    }

    struct logger_context_t
    {
      VW::io::logger& logger;
      std::string model_file_name;
    };

    VW::io::logger_output_func_t output_func = [](void* context, VW::io::log_level level, const std::string& message)
    {
      auto* logger_context = static_cast<logger_context_t*>(context);
      switch (level)
      {
        case VW::io::log_level::trace:
          logger_context->logger.info("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::debug:
          logger_context->logger.info("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::info:
          logger_context->logger.info("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::warn:
          logger_context->logger.warn("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::error:
          logger_context->logger.error("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::critical:
          logger_context->logger.critical("{}: {}", logger_context->model_file_name, message);
          break;
        case VW::io::log_level::off:
          break;
      }
    };

    std::vector<std::unique_ptr<VW::workspace>> models;
    std::vector<logger_context_t> logger_contexts;
    for (const auto& model_file : model_files)
    {
      logger.info("Loading model: {}", model_file);
      logger_contexts.push_back(logger_context_t{logger, model_file});
      auto model = VW::initialize_experimental(
          VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--driver_output_off", "--preserve_performance_counters"}),
          VW::io::open_file_reader(model_file), nullptr, nullptr, output_func, &logger_contexts.back());
      models.push_back(std::move(model));
    }

    // The first given model file is used as a template for the destination model.
    logger_contexts.push_back(logger_context_t{logger, model_files[0]});
    auto destination_model = VW::initialize_experimental(
        VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--driver_output_off", "--preserve_performance_counters"}),
        VW::io::open_file_reader(model_files[0]), nullptr, nullptr, output_func, &logger_contexts.back());

    // Let's do some sanity checks.
    // 1. are_features_compatible for each model
    // 2. Are the enabled reductions identical
    // 3. Are the command lines the same? Not sure if this is necessary...

    for (const auto& model : models)
    {
      auto* incompatible = VW::are_features_compatible(*destination_model, *model);
      if (incompatible != nullptr)
      {
        logger.error("Model is incompatible with the destination model. Reason: {}", incompatible);
        std::exit(1);
      }
    }

    std::vector<std::string> destination_enabled_reductions;
    destination_model->l->get_enabled_reductions(destination_enabled_reductions);
    for (const auto& model : models)
    {
      std::vector<std::string> source_enabled_reductions;
      model->l->get_enabled_reductions(source_enabled_reductions);

      if (source_enabled_reductions != destination_enabled_reductions)
      {
        logger.error("Enabled reductions are not identical between models.");
        std::exit(1);
      }
    }

    VW::config::cli_options_serializer serializer;
    for (auto const& option : destination_model->options->get_all_options())
    {
      if (destination_model->options->was_supplied(option->m_name)) { serializer.add(*option); }
    }
    auto destination_command_line = serializer.str();

    for (const auto& model : models)
    {
      VW::config::cli_options_serializer src_serializer;
      for (auto const& option : model->options->get_all_options())
      {
        if (model->options->was_supplied(option->m_name)) { src_serializer.add(*option); }
      }
      auto srd_cmdline = src_serializer.str();

      if (destination_command_line != srd_cmdline)
      {
        logger.error("Command lines are not identical between models.");
        logger.error(destination_command_line);
        logger.error(srd_cmdline);
        std::exit(1);
      }
    }

    std::vector<float> example_counts;
    example_counts.reserve(models.size());
    for (const auto& model : models) { example_counts.push_back(model->sd->weighted_labeled_examples); }

    std::vector<VW::workspace*> workspaces;
    workspaces.reserve(models.size());
    for (const auto& model : models) { workspaces.push_back(model.get()); }

    auto* target_learner = destination_model->l;
    while (target_learner != nullptr)
    {
      if (target_learner->has_merge())
      {
        std::vector<void*> reduction_data_per_model;
        for (const auto& model : models)
        {
          auto* source_data = model->l->get_learner_by_name_prefix(target_learner->get_name());
          reduction_data_per_model.push_back(source_data->get_internal_type_erased_data_pointer_test_use_only());
        }

        target_learner->merge(example_counts, workspaces, reduction_data_per_model, *destination_model,
            target_learner->get_internal_type_erased_data_pointer_test_use_only());
      }
      target_learner = target_learner->get_learn_base();
    }

    logger.info("Saving model: {}", output_file);
    VW::save_predictor(*destination_model, output_file);
  }
  catch (VW::vw_exception& e)
  {
    logger.critical("({}:{}): {}", e.Filename(), e.LineNumber(), e.what());
    return 1;
  }
  catch (std::exception& e)
  {
    logger.critical(e.what());
    return 1;
  }

  return 0;
}
