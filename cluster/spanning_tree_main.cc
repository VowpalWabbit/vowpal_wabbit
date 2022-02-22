/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This creates a binary tree topology over a set of n nodes that connect.

 */

#include "config/cli_help_formatter.h"
#include "config/option_builder.h"
#include "config/options_cli.h"
#include "config/option_group_definition.h"
#include "spanning_tree.h"
#include "vw_exception.h"

#ifdef _WIN32
int daemon(int a, int b) { return 0; }
int getpid() { return (int)::GetCurrentProcessId(); }
#endif

#include <iostream>
#include <fstream>

using namespace VW;

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
  bool help;

  VW::config::options_cli opts(std::vector<std::string>(argv + 1, argv + argc));

  VW::config::option_group_definition desc("Spanning Tree");
  desc.add(VW::config::make_option("nondaemon", nondaemon).help("Run spanning tree in foreground"));
  desc.add(VW::config::make_option("help", help).short_name('h').help("Print help message"));
  desc.add(VW::config::make_option("port", port).short_name('p').default_value(26543).help("Port number for spanning tree to listen on"));

  opts.add_and_parse(desc);
  // opts.check_unregistered();
  auto positional = opts.get_positional_tokens();
  std::string pid_file_name;
  if(!positional.empty())
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
      if (daemon(1, 1)) { THROWERRNO("daemon: "); }
    }

    SpanningTree spanningTree(port);

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

    spanningTree.Run();
  }
  catch (VW::vw_exception& e)
  {
    std::cerr << "spanning tree (" << e.Filename() << ":" << e.LineNumber() << "): " << e.what() << std::endl;
  }
}
