// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifdef _WIN32
#define NOMINMAX
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <sys/timeb.h>
#include "../../vowpalwabbit/parse_args.h"
#include "../../vowpalwabbit/parse_regressor.h"
#include "../../vowpalwabbit/accumulate.h"
#include "../../vowpalwabbit/best_constant.h"
#include "../../vowpalwabbit/vw_exception.h"
#include <fstream>

#include "../../vowpalwabbit/vw.h"
#include "../../vowpalwabbit/options.h"
#include "../../vowpalwabbit/options_boost_po.h"
#include "../../vowpalwabbit/parser/flatbuffer/vw_to_flat.h"

using namespace VW::config;

vw* setup(options_i& options)
{
  vw* all = nullptr;
  try
  {
    all = VW::initialize(options);
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    throw;
  }
  catch (...)
  {
    std::cout << "unknown exception" << std::endl;
    throw;
  }
  all->vw_is_main = true;

  if (!all->logger.quiet && !all->bfgs && !all->searchstr && !options.was_supplied("audit_regressor"))
  {
    all->trace_message << std::left << std::setw(shared_data::col_avg_loss) << std::left << "average"
                       << " " << std::setw(shared_data::col_since_last) << std::left << "since"
                       << " " << std::right << std::setw(shared_data::col_example_counter) << "example"
                       << " " << std::setw(shared_data::col_example_weight) << "example"
                       << " " << std::setw(shared_data::col_current_label) << "current"
                       << " " << std::setw(shared_data::col_current_predict) << "current"
                       << " " << std::setw(shared_data::col_current_features) << "current" << std::endl;
    all->trace_message << std::left << std::setw(shared_data::col_avg_loss) << std::left << "loss"
                       << " " << std::setw(shared_data::col_since_last) << std::left << "last"
                       << " " << std::right << std::setw(shared_data::col_example_counter) << "counter"
                       << " " << std::setw(shared_data::col_example_weight) << "weight"
                       << " " << std::setw(shared_data::col_current_label) << "label"
                       << " " << std::setw(shared_data::col_current_predict) << "predict"
                       << " " << std::setw(shared_data::col_current_features) << "features" << std::endl;
  }

  return all;
}

int main(int argc, char* argv[])
{
  bool should_use_onethread = false;
  option_group_definition driver_config("driver");
  driver_config.add(make_option("onethread", should_use_onethread).help("Disable parse thread"));
  std::vector<std::unique_ptr<options_boost_po>> arguments;
  std::vector<vw*> alls;
  if (argc == 3 && !std::strcmp(argv[1], "--args"))
  {
    std::fstream arg_file(argv[2]);
    if (!arg_file)
    {
      THROW("Could not open file: " << argv[2]);
    }

    int line_count = 1;
    std::string line;
    while (std::getline(arg_file, line))
    {
      std::stringstream sstr;
      sstr << line << " -f model." << (line_count++);
      sstr << " --no_stdin";  // can't use stdin with multiple models

      const std::string new_args = sstr.str();
      std::cout << new_args << std::endl;

      int l_argc;
      char** l_argv = VW::to_argv(new_args, l_argc);

      std::unique_ptr<options_boost_po> ptr(new options_boost_po(l_argc, l_argv));
      ptr->add_and_parse(driver_config);
      alls.push_back(setup(*ptr));
      arguments.push_back(std::move(ptr));
    }
  }
  else
  {
    std::unique_ptr<options_boost_po> ptr(new options_boost_po(argc, argv));
    ptr->add_and_parse(driver_config);
    alls.push_back(setup(*ptr));
    arguments.push_back(std::move(ptr));
  }

  vw& all = *alls[0];

  if (should_use_onethread)
  {
    if (alls.size() == 1){
      LEARNER::generic_driver_onethread(all);
      std::cout << "Examples number " << all.p->ready_parsed_examples.size() << "\n";
      VW::convert_txt_to_flat(all); 
    }
    else
      THROW("--onethread doesn't make sense with multiple learners");
  }
  else
  {
    VW::start_parser(all);
    VW::convert_txt_to_flat(all); 
    return 0;
    // if (alls.size() == 1)
    //   LEARNER::generic_driver(all);
    // else
    //   LEARNER::generic_driver(alls);
    VW::end_parser(all);
  }
  // }
}
