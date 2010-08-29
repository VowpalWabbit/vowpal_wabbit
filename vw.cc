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
#include "parse_regressor.h"
#include "parse_example.h"
#include "parse_args.h"
#include "gd.h"
#include "noop.h"
#include "vw.h"
#include "simple_label.h"
#include "sender.h"
#include "delay_ring.h"
#include "message_relay.h"

gd_vars* vw(int argc, char *argv[])
{
  size_t numpasses;
  float eta_decay;
  ofstream final_regressor;
  string final_regressor_name;

  parser* p = new_parser(&simple_label);
  regressor regressor1;

  gd_vars *vars = (gd_vars*) malloc(sizeof(gd_vars));

  po::options_description desc("VW options");

  po::variables_map vm = parse_args(argc, argv, desc, *vars, eta_decay, 
				    numpasses, regressor1, p, 
				    final_regressor_name);

  if (!global.quiet)
    {
      const char * header_fmt = "%-10s %-10s %8s %8s %10s %8s %8s\n";
      fprintf(stderr, header_fmt,
	      "average", "since", "example", "example",
	      "current", "current", "current");
      fprintf(stderr, header_fmt,
	      "loss", "last", "counter", "weight", "label", "predict", "features");
      cerr.precision(5);
    }

  size_t num_threads = global.num_threads();
  gd_thread_params t = {vars, num_threads, regressor1, &final_regressor_name};
  
  for (; numpasses > 0; numpasses--) {
    start_parser(num_threads, p);
    initialize_delay_ring();

    if (global.local_prediction > 0 && (global.unique_id == 0 || global.backprop) )
      setup_relay();
    
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
    else
      {
	setup_gd(t);
	destroy_gd();
      }
    
    if (global.unique_id == 0 && global.local_prediction > 0)
      destroy_relay();
    
    destroy_delay_ring();
    end_parser(p);
    vars->eta *= eta_decay;
    reset_source(global.num_bits, p);
  }
  
  if (final_regressor_name  != "")
    {
      final_regressor.open(final_regressor_name.c_str());
      dump_regressor(final_regressor, regressor1);
    }
  finalize_regressor(final_regressor,regressor1);
  finalize_source(p);
  free(p);
  
  return vars;
}
