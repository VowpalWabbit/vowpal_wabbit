// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifdef _WIN32
#  define NOMINMAX
#  include <WinSock2.h>
#else
#  include <sys/socket.h>
#  include <arpa/inet.h>
#endif
#include <sys/timeb.h>
#include "parse_args.h"
#include "parse_regressor.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_exception.h"
#include <fstream>

#include "config/options.h"
#include "config/options_boost_po.h"
#include "vw_to_flat.h"

using namespace VW::config;

VW::workspace* setup(std::unique_ptr<options_i, options_deleter_type> options)
{
  VW::workspace* all = nullptr;
  try
  {
    all = VW::initialize(std::move(options));
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

  if (!all->quiet && !all->bfgs && !all->searchstr && !all->options->was_supplied("audit_regressor"))
  {
    *(all->trace_message) << std::left << std::setw(shared_data::col_avg_loss) << std::left << "average"
                          << " " << std::setw(shared_data::col_since_last) << std::left << "since"
                          << " " << std::right << std::setw(shared_data::col_example_counter) << "example"
                          << " " << std::setw(shared_data::col_example_weight) << "example"
                          << " " << std::setw(shared_data::col_current_label) << "current"
                          << " " << std::setw(shared_data::col_current_predict) << "current"
                          << " " << std::setw(shared_data::col_current_features) << "current" << std::endl;
    *(all->trace_message) << std::left << std::setw(shared_data::col_avg_loss) << std::left << "loss"
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
  option_group_definition driver_config("Driver");

  to_flat converter;
  driver_config.add(make_option("fb_out", converter.output_flatbuffer_name));
  driver_config.add(make_option("collection_size", converter.collection_size));

  std::vector<VW::workspace*> alls;

  std::string q("--quiet");
  argv[argc++] = const_cast<char*>(q.c_str());

  std::unique_ptr<options_boost_po, options_deleter_type> ptr(
      new options_boost_po(argc, argv), [](VW::config::options_i* ptr) { delete ptr; });
  ptr->add_and_parse(driver_config);
  alls.push_back(setup(std::move(ptr)));
  if (converter.collection_size > 0) { converter.collection = true; }

  VW::workspace& all = *alls[0];

  VW::start_parser(all);
  converter.convert_txt_to_flat(all);
  VW::end_parser(all);
  return 0;
}
