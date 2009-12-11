/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "vw.h"
#include "gd.h"

int main(int argc, char *argv[]) {
	gd_vars *vars = vw(argc, argv);

	float best_constant = global.weighted_labels / global.weighted_examples;
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
		if (vars->min_prediction == 0. && vars->max_prediction == 1. && best_constant < 1. && best_constant > 0.)
		  cerr << endl << "best constant's loss = " << constant_loss;
		cerr << endl << "total feature number = " << global.total_features;
		cerr << endl;
	}

	delete vars;

	return 0;
}
