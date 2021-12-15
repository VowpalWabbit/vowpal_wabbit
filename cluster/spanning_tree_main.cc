/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This creates a binary tree topology over a set of n nodes that connect.

 */

#include "spanning_tree.h"
#include "vw_exception.h"

#if defined(_WIN32) || defined(__APPLE__)
int daemon(int a, int b) { return 0; }
#endif
#ifdef _WIN32
int getpid() { return (int)::GetCurrentProcessId(); }
#endif

#include <iostream>
#include <fstream>

using namespace VW;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

void usage(const po::options_description& desc)
{
  std::cout << "usage: spanning_tree [--port,-p number] [--nondaemon] [--help,-h] [pid_file]" << std::endl;
  std::cout << desc << std::endl;
}

int main(int argc, char* argv[])
{
  int port = 26543;
  bool nondaemon = false;

  po::variables_map vm;
  po::options_description desc("Spanning Tree");
  desc.add_options()("nondaemon", po::bool_switch(&nondaemon), "Run spanning tree in foreground")(
      "help,h", "Print help message")("port,p", po::value<int>(&port), "Port number for spanning tree to listen on");

  std::string pid_file_name;
  po::options_description hidden;
  hidden.add_options()("pid_file", po::value<std::string>(&pid_file_name), "File to write PID value to.");

  po::options_description all_options;
  all_options.add(desc);
  all_options.add(hidden);

  po::positional_options_description pos;
  pos.add("pid_file", -1);

  try
  {
    po::store(po::command_line_parser(argc, argv).options(all_options).positional(pos).run(), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cout << argv[0] << ": " << e.what() << std::endl << std::endl;
    usage(desc);
    return 1;
  }

  if (vm.count("help"))
  {
    usage(desc);
    return 0;
  }

  try
  {
    if (!nondaemon)
    {
      if (daemon(1, 1)) { THROWERRNO("daemon: "); }
    }

    SpanningTree spanningTree(port);

    if (vm.count("pid_file"))
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
