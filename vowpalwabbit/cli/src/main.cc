// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

#include <fstream>

using namespace VW::config;

std::unique_ptr<VW::workspace> setup(std::unique_ptr<options_i> options)
{
  auto all = VW::initialize(std::move(options));
  all->vw_is_main = true;
  return all;
}

int main(int argc, char* argv[])
{
  bool should_use_onethread = false;
  std::string log_level;
  std::string log_output_stream;
  option_group_definition driver_config("Driver");
  driver_config.add(make_option("onethread", should_use_onethread).help("Disable parse thread"));
  driver_config.add(make_option("log_level", log_level)
                        .default_value("info")
                        .hidden()
                        .one_of({"info", "warn", "error", "critical", "off"})
                        .help("Log level for logging messages. Specifying this wil override --quiet for log output"));
  driver_config.add(make_option("log_output", log_output_stream)
                        .default_value("stdout")
                        .hidden()
                        .one_of({"stdout", "stderr", "compat"})
                        .help("Specify the stream to output log messages to. In the past VW's choice of stream for "
                              "logging messages wasn't consistent. Supplying compat will maintain that old behavior. "
                              "Compat is now deprecated so it is recommended that stdout or stderr is chosen"));

  auto main_logger = VW::io::create_default_logger();
  try
  {
    // support multiple vw instances for training of the same datafile for the same instance
    std::vector<std::unique_ptr<VW::workspace>> alls;
    if (argc == 3 && !std::strcmp(argv[1], "--args"))
    {
      std::fstream arg_file(argv[2]);
      if (!arg_file) { THROW("Could not open file: " << argv[2]); }

      int line_count = 1;
      std::string line;
      while (std::getline(arg_file, line))
      {
        std::stringstream sstr;
        sstr << line << " -f model." << (line_count++);
        sstr << " --no_stdin";  // can't use stdin with multiple models

        const std::string new_args = sstr.str();
        std::cout << new_args << std::endl;

        auto ptr = VW::make_unique<options_cli>(VW::split_command_line(new_args));
        ptr->add_and_parse(driver_config);
        auto level = VW::io::get_log_level(log_level);
        main_logger.set_level(level);
        auto location = VW::io::get_output_location(log_output_stream);
        main_logger.set_location(location);
        alls.push_back(setup(std::move(ptr)));
      }
    }
    else
    {
      auto ptr = VW::make_unique<options_cli>(std::vector<std::string>(argv + 1, argv + argc));
      ptr->add_and_parse(driver_config);
      auto level = VW::io::get_log_level(log_level);
      main_logger.set_level(level);
      auto location = VW::io::get_output_location(log_output_stream);
      main_logger.set_location(location);
      alls.push_back(setup(std::move(ptr)));
    }

    VW::workspace& all = *alls[0];

    auto skip_driver = all.options->get_typed_option<bool>("dry_run").value();

    if (skip_driver)
    {
      // Leave deletion up to the unique_ptr
      for (auto& v : alls) { v->finish(); }
      return 0;
    }

    if (should_use_onethread)
    {
      if (alls.size() == 1) { VW::LEARNER::generic_driver_onethread(all); }
      else
        THROW("--onethread doesn't make sense with multiple learners");
    }
    else
    {
      VW::start_parser(all);
      if (alls.size() == 1) { VW::LEARNER::generic_driver(all); }
      else
      {
        std::vector<VW::workspace*> alls_ptrs;
        alls_ptrs.reserve(alls.size());
        for (auto& v : alls) { alls_ptrs.push_back(v.get()); }
        VW::LEARNER::generic_driver(alls_ptrs);
      }
      VW::end_parser(all);
    }

    for (auto& v : alls)
    {
      if (v->example_parser->exc_ptr) { std::rethrow_exception(v->example_parser->exc_ptr); }

      VW::sync_stats(*v);
      // Leave deletion up to the unique_ptr
      v->finish();
    }
  }
  catch (VW::vw_exception& e)
  {
    main_logger.err_critical("vw ({}:{}): {}", e.filename(), e.line_number(), e.what());
    return 1;
  }
  catch (std::exception& e)
  {
    main_logger.err_critical("vw: {}", e.what());
    return 1;
  }
  catch (...)
  {
    main_logger.err_critical("vw: unknown exception");
    return 1;
  }

  return 0;
}
