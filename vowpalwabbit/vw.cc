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
#include "global_data.h"
#include "parse_example.h"
#include "parse_args.h"
#include "accumulate.h"

using namespace std;

int main(int argc, char *argv[])
{
  srand48(0);

  parse_args(argc, argv);
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

  start_parser(global.p);

  global.driver();

  end_parser(global.p);
  
  finalize_regressor(global.final_regressor_name,global.reg);
  finalize_source(global.p);
  free(global.p);
  ftime(&t_end);
  double net_time = (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
  if(!global.quiet && global.span_server != "")
    cerr<<"Net time taken by process = "<<net_time/(double)(1000)<<" seconds\n";

  if(global.span_server != "") {
    float loss = global.sd->sum_loss;
    global.sd->sum_loss = (double)accumulate_scalar(global.span_server, loss);
    float weighted_examples = global.sd->weighted_examples;
    global.sd->weighted_examples = (double)accumulate_scalar(global.span_server, weighted_examples);
    float weighted_labels = global.sd->weighted_labels;
    global.sd->weighted_labels = (double)accumulate_scalar(global.span_server, weighted_labels);
    float weighted_unlabeled_examples = global.sd->weighted_unlabeled_examples;
    global.sd->weighted_unlabeled_examples = (double)accumulate_scalar(global.span_server, weighted_unlabeled_examples);
    float example_number = global.sd->example_number;
    global.sd->example_number = (uint64_t)accumulate_scalar(global.span_server, example_number);
    float total_features = global.sd->total_features;
    global.sd->total_features = (uint64_t)accumulate_scalar(global.span_server, total_features);
  }

  float weighted_labeled_examples = global.sd->weighted_examples - global.sd->weighted_unlabeled_examples;
  float best_constant = (global.sd->weighted_labels - global.initial_t) / weighted_labeled_examples;
  float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
			 + (1.0 - best_constant)*best_constant*best_constant);
  
  if (!global.quiet)
    {
      cerr.precision(4);
      cerr << endl << "finished run";
      cerr << endl << "number of examples = " << global.sd->example_number;
      cerr << endl << "weighted example sum = " << global.sd->weighted_examples;
      cerr << endl << "weighted label sum = " << global.sd->weighted_labels;
      cerr << endl << "average loss = " << global.sd->sum_loss / global.sd->weighted_examples;
      cerr << endl << "best constant = " << best_constant;
      if (global.sd->min_label == 0. && global.sd->max_label == 1. && best_constant < 1. && best_constant > 0.)
	cerr << endl << "best constant's loss = " << constant_loss;
      cerr << endl << "total feature number = " << global.sd->total_features;
      if (global.active_simulation)
	cerr << endl << "total queries = " << global.sd->queries << endl;
      cerr << endl;
    }
  
  free(global.sd);
  free(global.lp);
  delete global.loss;
  
  return 0;
}
