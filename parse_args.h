/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PA_H
#define PA_H

void parse_args(int argc, char *argv[], float &eta, float &eta_decay_rate, float &initial_t, 
		float &power_t, ofstream &predictions, ofstream &raw_predictions, bool &train, 
		int &numthreads, int& passes, regressor &r, example_file &e,
		ofstream &final_regressor);
#endif
