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
#include "vw_to_flat.h"

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
  bool should_use_onethread;
  option_group_definition driver_config("driver");
  std::vector<std::unique_ptr<options_boost_po>> arguments;
  std::vector<vw*> alls;
  char *newargs[argc+1];
  char *quiet = "--quiet";
  // memcpy(&newargs, &argv, argc);
  for(int j = 0; j<argc; j++)
    {
      newargs[j] = argv[j];
    }
  newargs[argc] = quiet;
  // std::cout << newargs[0] << " " << newargs[1] << " " << newargs[argc];
  std::unique_ptr<options_boost_po> ptr(new options_boost_po(argc+1, newargs));
  ptr->add_and_parse(driver_config);
  alls.push_back(setup(*ptr));
  arguments.push_back(std::move(ptr));

  vw& all = *alls[0];

  VW::start_parser(all);
  all.logger.quiet = true;
  convert_txt_to_flat(all);
  VW::end_parser(all); 
  return 0;
}
