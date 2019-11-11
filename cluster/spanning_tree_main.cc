/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This creates a binary tree topology over a set of n nodes that connect.

 */

#include "spanning_tree.h"
#include "vw_exception.h"

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

int main(int argc, char* argv[])
{
  if (argc > 2)
  {
    std::cout << "usage: spanning_tree [--nondaemon | pid_file]" << std::endl;
    exit(0);
  }

  try
  {
    if (argc == 2 && strcmp("--nondaemon", argv[1]) == 0)
      ;
    else if (daemon(1, 1))
      THROWERRNO("daemon: ");

    SpanningTree spanningTree;

    if (argc == 2 && strcmp("--nondaemon", argv[1]) != 0)
    {
      std::ofstream pid_file;
      pid_file.open(argv[1]);
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
