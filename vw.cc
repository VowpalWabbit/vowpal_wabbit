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
#include "vw.h"
#include "sender.h"

int sum_sock = -1;

gd_vars* vw(int argc, char *argv[])
{
  size_t numpasses;
  float eta_decay;
  ofstream final_regressor;
  string final_regressor_name;

  example_source source;
  parser* p = new_parser(&source,&simple_label);
  regressor regressor1;

  gd_vars *vars = new gd_vars;

  po::options_description desc("VW options");

  string comment;
  po::variables_map vm = parse_args(argc, argv, desc, *vars, eta_decay, 
				    numpasses, regressor1, p, 
				    final_regressor_name, sum_sock, comment);

  if (!vars->quiet)
    {
      cerr << "average\tsince\texample\texample\tcurrent\tcurrent\tcurrent" << endl;
      cerr << "loss\tlast\tcounter\tweight\tlabel\tpredict\tfeatures" << endl;
      cerr.precision(4);
    }

  size_t num_threads = regressor1.global->num_threads();
  gd_thread_params t = {vars, num_threads, regressor1, &final_regressor_name};
  
  for (; numpasses > 0; numpasses--) {
    setup_parser(num_threads, p);
    if (vm.count("sendto"))
      {
	setup_send();
	destroy_send();
      }
    else
      {
	setup_gd(t);
	destroy_gd();
      }
    destroy_parser(p);
    vars->eta *= eta_decay;
    if (numpasses > 1)
      reset_source(regressor1.global->num_bits, source);
  }
  
  if (final_regressor_name  != "")
    {
      final_regressor.open(final_regressor_name.c_str());
      dump_regressor(final_regressor, regressor1);
    }
  
  free(p);
  finalize_regressor(final_regressor,regressor1);
  finalize_source(source);
  source.global->pairs.~vector();
  free(source.global);
  
  return vars;
}
