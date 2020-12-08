/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This creates a binary tree topology over a set of n nodes that connect.

 */

#include "spanning_tree.h"
#include "vw_exception.h"

#include "options_boost_po.h"

#ifdef _WIN32
int daemon(int a, int b) { return 0; }
int getpid() { return (int)::GetCurrentProcessId(); }
#endif

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

using namespace VW;

void usage(const VW::config::options_boost_po& options)
{
  std::cout << "usage: spanning_tree [--port number] [--nondaemon] [--help] [pid_file]" << std::endl;
  std::cout << options.help() << std::endl;
  exit(0);
}

int main(int argc, char* argv[])
{
  int port;
  bool nondaemon = false;
  bool help = false;
  VW::config::options_boost_po options(argc, argv);
  VW::config::option_group_definition spanning_tree_options_group("Spanning Tree");
  spanning_tree_options_group.add(
      VW::config::make_option("port", port).default_value(26543).help("Port number for spanning tree to listen on"));
  spanning_tree_options_group.add(
      VW::config::make_option("nondaemon", nondaemon).help("Run spanning tree in foreground"));
  spanning_tree_options_group.add(VW::config::make_option("help", help).help("Print help message"));
  options.add_and_parse(spanning_tree_options_group);
  options.check_unregistered();

  if (help) { usage(options); }

  auto positional_tokens = options.get_positional_tokens();
  std::string pid_file_name = "";
  if (positional_tokens.size() == 1) { pid_file_name = positional_tokens[0]; }
  else if (positional_tokens.size() > 1)
  {
    usage(options);
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
        exit(1);
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
