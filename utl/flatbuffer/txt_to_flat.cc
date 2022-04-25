// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#ifdef _WIN32
#  define NOMINMAX
#  include <WinSock2.h>
#else
#  include <arpa/inet.h>
#  include <sys/socket.h>
#endif
#include "vw/common/vw_exception.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/accumulate.h"
#include "vw/core/best_constant.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_regressor.h"
#include "vw_to_flat.h"

#include <sys/timeb.h>

#include <fstream>

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
  { all->sd->print_update_header(*(all->trace_message)); }

  return all;
}

int main(int argc, char* argv[])
{
  option_group_definition driver_config("Driver");

  to_flat converter;
  driver_config.add(make_option("fb_out", converter.output_flatbuffer_name));
  driver_config.add(make_option("collection_size", converter.collection_size));

  std::vector<VW::workspace*> alls;

  std::vector<std::string> opts(argv + 1, argv + argc);
  opts.emplace_back("--quiet");

  std::unique_ptr<options_cli, options_deleter_type> ptr(
      new options_cli(opts), [](VW::config::options_i* ptr) { delete ptr; });
  ptr->add_and_parse(driver_config);
  alls.push_back(setup(std::move(ptr)));
  if (converter.collection_size > 0) { converter.collection = true; }

  VW::workspace& all = *alls[0];

  VW::start_parser(all);
  converter.convert_txt_to_flat(all);
  VW::end_parser(all);
  return 0;
}
