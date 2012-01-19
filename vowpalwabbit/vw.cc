/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include "parse_regressor.h"
#include "parse_example.h"
#include "parse_args.h"
#include "gd.h"
#include "gd_mf.h"
#include "lda_core.h"
#include "bfgs.h"
#include "lda_core.h"
#include "noop.h"
#include "vw.h"
#include "sender.h"
#include "allreduce.h"

using namespace std;

void vw(int argc, char *argv[])
{
  srand48(0);

  parser* p = new_parser();
  
  po::options_description desc("VW options");
  
  po::variables_map vm = parse_args(argc, argv, desc, p);
  struct timeb t_start, t_end;
  ftime(&t_start);
  
  if (!global.quiet && !global.bfgs && !global.sequence)
    {
      const char * header_fmt = "%-10s %-10s %8s %8s %10s %8s %8s\n";
      fprintf(stderr, header_fmt,
	      "average", "since", "example", "example",
	      "current", "current", "current");
      fprintf(stderr, header_fmt,
	      "loss", "last", "counter", "weight", "label", "predict", "features");
      cerr.precision(5);
    }

  start_parser(p);

  global.driver();

  end_parser(p);
  
  finalize_regressor(global.final_regressor_name,global.reg);
  finalize_source(p);
  free(p);
  ftime(&t_end);
  double net_time = (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  if(!global.quiet && global.span_server != "")
    cerr<<"Net time taken by process = "<<net_time/(double)(1000)<<" seconds\n";
}
