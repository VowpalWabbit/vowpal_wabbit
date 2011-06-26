#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <pthread.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "example.h"
#include "delay_ring.h"
#include "parse_args.h"
#include "simple_label.h"
#include "gd.h"
#include "lda_core.h"

int main(int argc, char *argv[])
{
  string final_regressor_name;

  parser* p = new_parser(&simple_label);
  regressor regressor1;

  gd_vars *vars = (gd_vars*) malloc(sizeof(gd_vars));

  po::options_description desc("VW options");

  po::variables_map vm = parse_args(argc, argv, desc, *vars, 
				    regressor1, p, 
				    final_regressor_name);
  
  if (!global.quiet && !vm.count("conjugate_gradient"))
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

  start_parser(num_threads, p);
  initialize_delay_ring();
  start_lda(t);
  end_lda();

  destroy_delay_ring();
  end_parser(p);

  finalize_regressor(final_regressor_name,regressor1);
  finalize_source(p);
  free(p);

  float weighted_labeled_examples = global.weighted_examples - global.weighted_unlabeled_examples;
  float best_constant = (global.weighted_labels - global.initial_t) / weighted_labeled_examples;
  float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
			 + (1.0 - best_constant)*best_constant*best_constant);
  
  if (!global.quiet)
    {
      cerr.precision(4);
      cerr << endl << "finished run";
      cerr << endl << "number of examples = " << global.example_number;
      cerr << endl << "weighted example sum = " << global.weighted_examples;
      cerr << endl << "weighted label sum = " << global.weighted_labels;
      cerr << endl << "average loss = " << global.sum_loss / global.weighted_examples;
      cerr << endl << "best constant = " << best_constant;
      if (global.min_label == 0. && global.max_label == 1. && best_constant < 1. && best_constant > 0.)
	cerr << endl << "best constant's loss = " << constant_loss;
      cerr << endl << "total feature number = " << global.total_features;
      if (global.active_simulation)
	cerr << endl << "total queries = " << global.queries << endl;
      cerr << endl;
    }
  
  return 0;
}
