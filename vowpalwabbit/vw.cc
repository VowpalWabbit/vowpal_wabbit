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

gd_vars* vw(int argc, char *argv[])
{
  string final_regressor_name;
  srand48(0);

  parser* p = new_parser();
  regressor regressor1;
  
  gd_vars *vars = (gd_vars*) malloc(sizeof(gd_vars));

  po::options_description desc("VW options");
  
  po::variables_map vm = parse_args(argc, argv, desc, *vars, 
				    regressor1, p, 
				    final_regressor_name);
  struct timeb t_start, t_end;
  ftime(&t_start);
  
  if (!global.quiet && !global.bfgs)
    {
      const char * header_fmt = "%-10s %-10s %8s %8s %10s %8s %8s\n";
      fprintf(stderr, header_fmt,
	      "average", "since", "example", "example",
	      "current", "current", "current");
      fprintf(stderr, header_fmt,
	      "loss", "last", "counter", "weight", "label", "predict", "features");
      cerr.precision(5);
    }

  gd_thread_params t = {vars, regressor1, &final_regressor_name};

  start_parser(p);

  if (vm.count("sendto"))
    {
      setup_send();
      destroy_send();
    }
  else if (vm.count("noop"))

    {
      start_noop();
      end_noop();
    }
  else if (global.bfgs)
    {
      BFGS::setup_bfgs(t);
      BFGS::destroy_bfgs();
    }
  else if (global.rank > 0)
    {
      setup_gd_mf(t);
      destroy_gd_mf();
    } 
  else if (global.lda > 0)
    {
      start_lda(t);
      end_lda();
    }
  else 
    {
      setup_gd(t);
      destroy_gd();
    }

  end_parser(p);
  
  finalize_regressor(final_regressor_name,t.reg);
  finalize_source(p);
  free(p);
  ftime(&t_end);
  double net_time = (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  if(!global.quiet && global.span_server != "")
    cerr<<"Net time taken by process = "<<net_time/(double)(1000)<<" seconds\n";
  return vars;
}
