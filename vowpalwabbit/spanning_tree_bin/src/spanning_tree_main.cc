/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This creates a binary tree topology over a set of n nodes that connect.

 */

#include "vw/common/vw_exception.h"
#include "vw/config/cli_help_formatter.h"
#include "vw/config/option_builder.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options_cli.h"
#include "vw/io/errno_handling.h"
#include "vw/spanning_tree/spanning_tree.h"

#ifdef _WIN32
int daemon(int /*a*/, int /*b*/) { return 0; }
int getpid() { return (int)::GetCurrentProcessId(); }
#endif

#include <fstream>
#include <iostream>

void usage(const VW::config::options_cli& desc)
{
  std::cout << "usage: spanning_tree [--port,-p number] [--nondaemon] [--help,-h] [pid_file]" << std::endl;
  VW::config::cli_help_formatter help_formatter;
  std::cout << help_formatter.format_help(desc.get_all_option_group_definitions()) << std::endl;
}

int main(int argc, char* argv[])
{
  int port = 26543;
  bool nondaemon = false;
  bool help = false;

  VW::config::options_cli opts(std::vector<std::string>(argv + 1, argv + argc));

  VW::config::option_group_definition desc("Spanning Tree");
  desc.add(VW::config::make_option("nondaemon", nondaemon).help("Run spanning tree in foreground"))
      .add(VW::config::make_option("help", help).short_name("h").help("Print help message"))
      .add(VW::config::make_option("port", port)
               .short_name("p")
               .default_value(26543)
               .help("Port number for spanning tree to listen on"));

  opts.add_and_parse(desc);
  // Return value is ignored as option reachability is not relevant here.
  auto warnings = opts.check_unregistered();
  _UNUSED(warnings);
  auto positional = opts.get_positional_tokens();
  std::string pid_file_name;
  if (!positional.empty())
  {
    pid_file_name = positional.front();
    if (positional.size() > 1)
    {
      std::cerr << "Too many positional arguments" << std::endl;
      usage(opts);
      return 1;
    }
  }

  if (help)
  {
    usage(opts);
    return 0;
  }

  try
  {
    if (!nondaemon)
    {
      VW_WARNING_STATE_PUSH
      VW_WARNING_DISABLE_DEPRECATED_USAGE
      if (daemon(1, 1)) { THROWERRNO("daemon: "); }
      VW_WARNING_STATE_POP
    }

    VW::spanning_tree spanning_tree_obj(static_cast<short unsigned int>(port));

    if (!pid_file_name.empty())
    {
      std::ofstream pid_file;
      pid_file.open(pid_file_name);
      if (!pid_file.is_open())
      {
        std::cerr << "error writing pid file" << std::endl;
        return 1;
      }
      pid_file << getpid() << std::endl;
      pid_file.close();
    }

    spanning_tree_obj.run();
  }
  catch (VW::vw_exception& e)
  {
    std::cerr << "spanning tree (" << e.filename() << ":" << e.line_number() << "): " << e.what() << std::endl;
  }
}
