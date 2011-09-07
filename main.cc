/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include <stdlib.h>
#include "vw.h"
#include "gd.h"
#include "accumulate.h"

using namespace std;

int main(int argc, char *argv[]) {
  gd_vars *vars = vw(argc, argv);

  if(global.span_server != "") {
    float loss = global.sum_loss;
    global.sum_loss = (double)accumulate_scalar(global.span_server, loss);
    float weighted_examples = global.weighted_examples;
    global.weighted_examples = (double)accumulate_scalar(global.span_server, weighted_examples);
    float weighted_labels = global.weighted_labels;
    global.weighted_labels = (double)accumulate_scalar(global.span_server, weighted_labels);
    float weighted_unlabeled_examples = global.weighted_unlabeled_examples;
    global.weighted_unlabeled_examples = (double)accumulate_scalar(global.span_server, weighted_unlabeled_examples);
    float example_number = global.example_number;
    global.example_number = (uint64_t)accumulate_scalar(global.span_server, example_number);
    float total_features = global.total_features;
    global.total_features = (uint64_t)accumulate_scalar(global.span_server, total_features);
  }

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
  
  free(vars);
  
  return 0;
}
