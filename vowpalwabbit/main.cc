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

#include "vw.h"
#include "options.h"
#include "options_boost_po.h"

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
    throw;
  }
  catch (...)
  {
    std::cout << "unknown exception" << std::endl;
    throw;
  }
  all->vw_is_main = true;

  return all;
}

int main(int argc, char* argv[])
{
  bool should_use_onethread = false;
  option_group_definition driver_config("driver");
  driver_config.add(make_option("onethread", should_use_onethread).help("Disable parse thread"));

  try
  {
    // support multiple vw instances for training of the same datafile for the same instance
    std::vector<std::unique_ptr<options_boost_po>> arguments;
    std::vector<vw*> alls;
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

    auto skip_driver = all.options->get_typed_option<bool>("dry_run").value();

    if (skip_driver)
    {
      for (vw* v : alls) { VW::finish(*v); }
      return 0;
    }

    if (should_use_onethread)
    {
      if (alls.size() == 1)
        VW::LEARNER::generic_driver_onethread(all);
      else
        THROW("--onethread doesn't make sense with multiple learners");
    }
    else
    {
      VW::start_parser(all);
      if (alls.size() == 1)
        VW::LEARNER::generic_driver(all);
      else
        VW::LEARNER::generic_driver(alls);
      VW::end_parser(all);
    }

    for (vw* v : alls)
    {
      if (v->example_parser->exc_ptr) { std::rethrow_exception(v->example_parser->exc_ptr); }

      VW::sync_stats(*v);
      VW::finish(*v);
    }
  }
  catch (VW::vw_exception& e)
  {
    // TODO: If loggers are instantiated within struct vw, this line lives outside of that. Log as critical for now
    std::cerr << "[critical] vw (" << e.Filename() << ":" << e.LineNumber() << "): " << e.what() << std::endl;
    return 1;
  }
  catch (std::exception& e)
  {
    // vw is implemented as a library, so we use 'throw runtime_error()'
    // error 'handling' everywhere.  To reduce stderr pollution
    // everything gets caught here & the error message is printed
    // sans the excess exception noise, and core dump.
    // TODO: If loggers are instantiated within struct vw, this line lives outside of that. Log as critical for now
    std::cerr << "[critical] vw: " << e.what() << std::endl;
    // cin.ignore();
    return 1;
  }
  // cin.ignore();
  return 0;
}
