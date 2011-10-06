/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef LOSSFUNCTIONS_H_
#define LOSSFUNCTIONS_H_

#include <string>

class loss_function {

public :
	/*
	 * getLoss evaluates the example loss.
	 * The function returns the loss value
	 */
	//virtual float getLoss(example *&ec, gd_vars &vars) = 0;
	virtual float getLoss(float prediction, float label) = 0;

	/*
	 * getUpdate evaluates the update scalar
	 * The function return the update scalar
	 */
	virtual float getUpdate(float prediction, float label, float eta_t, float norm) = 0;
	virtual float getRevertingWeight(float prediction, float eta_t) = 0;
	virtual float getSquareGrad(float prediction, float label) = 0;
	virtual float first_derivative(float prediction, float label) = 0;
	virtual float second_derivative(float prediction, float label) = 0;
	virtual ~loss_function() {};
};

loss_function* getLossFunction(std::string funcName, double loss_parameter = 0);

#endif /* LOSSFUNCTIONS_H_ */
