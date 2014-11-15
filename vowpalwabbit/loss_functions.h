/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <string>
#include "parse_primitives.h"

struct shared_data;

using namespace std;

class loss_function {

public :
	/*
	 * getLoss evaluates the example loss.
	 * The function returns the loss value
	 */
	//virtual float getLoss(example *&ec, gd_vars &vars) = 0;
  virtual float getLoss(shared_data*, float prediction, float label) = 0;

	/*
	 * getUpdate evaluates the update scalar
	 * The function return the update scalar
	 */
	virtual float getUpdate(float prediction, float label, float eta_t, float norm) = 0;
	virtual float getUnsafeUpdate(float prediction, float label, float eta_t, float norm) = 0;
	virtual float getRevertingWeight(shared_data*, float prediction, float eta_t) = 0;
	virtual float getSquareGrad(float prediction, float label) = 0;
	virtual float first_derivative(shared_data*, float prediction, float label) = 0;
	virtual float second_derivative(shared_data*, float prediction, float label) = 0;
	virtual ~loss_function() {};
};

loss_function* getLossFunction(void*, string funcName, float function_parameter = 0);
