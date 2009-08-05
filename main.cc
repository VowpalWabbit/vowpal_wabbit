/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "vw.h"
#include "gd.h"


int main(int argc, char *argv[]) 
{
  gd_vars *vars = vw(argc, argv);
  
  float best_constant = vars->weighted_labels / vars->weighted_examples;
  float constant_loss = (best_constant*(1.0 - best_constant)*(1.0 - best_constant)
			 + (1.0 - best_constant)*best_constant*best_constant);
  
  if (!vars->quiet)
    {
      cerr.precision(4);
      cerr << endl << "finished run";
      cerr << endl << "number of examples = " << vars->example_number;
      cerr << endl << "weighted example sum = " << vars->weighted_examples;
      cerr << endl << "weighted label sum = " << vars->weighted_labels;
      cerr << endl << "average loss = " << vars->sum_loss / vars->weighted_examples;
      cerr << endl << "best constant's loss = " << constant_loss;
      cerr << endl << "total feature number = " << vars->total_features;
      cerr << endl;
    }
  delete vars;
  
  return 0;
}
