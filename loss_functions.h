/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef LOSSFUNCTIONS_H_
#define LOSSFUNCTIONS_H_

#include <string>
using namespace std;

class loss_function {

public :
	/*
	 * getLoss evaluates the example loss.
	 * The function returns the loss value
	 */
	//virtual double getLoss(example *&ec, gd_vars &vars) = 0;
	virtual double getLoss(double prediction, double label) = 0;

	/*
	 * getUpdate evaluates the update scalar
	 * The function return the update scalar
	 */
	virtual double getUpdate(double prediction, double label, double eta_t, double norm, float h) = 0;
	virtual ~loss_function() {};
};

loss_function* getLossFunction(string funcName, double loss_parameter = 0);

#endif /* LOSSFUNCTIONS_H_ */
