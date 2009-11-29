/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "loss_functions.h"
#include<math.h>
#include<iostream>

class squaredloss : public loss_function {
public:
	squaredloss() {

	}

	double getLoss(double prediction, double label) {
		double example_loss = (prediction - label) * (prediction - label);
		return example_loss;
	}

	double getUpdate(double prediction, double label) {
		return (label - prediction);
	}
};

class hingeloss : public loss_function {
public:
	hingeloss() {

	}

	double getLoss(double prediction, double label) {
		double e = 1 - label*prediction;
		return (e > 0) ? e : 0;
	}

	double getUpdate(double prediction, double label) {
		double y = (label == 0.0) ? -1.0 : 1.0;
		if(prediction == y) return 0;
		if((1 - y*prediction) > 0)
			return y;
		else 
			return 0;
	}
};

class logloss : public loss_function {
public:
	logloss() {

	}

	double getLoss(double prediction, double label) {
		double y = (label == 0.0) ? -1.0 : 1.0;
		return log(1 + exp(-y * prediction));
	}

	double getUpdate(double prediction, double label) {
		double y = (label == 0.0) ? -1.0 : 1.0;
		double d = exp(-y * prediction);
		return y * d / (1 + d);
	}
};

class quantilesloss : public loss_function {
public:
	quantilesloss(double &tau_) : tau(tau_) {
	}

	double getLoss(double prediction, double label) {
		double y = (label == 0.0) ? -1.0 : 1.0;
		double e = y - prediction;
		if(e > 0) {
			return tau * e;
		} else {
			return -(1 - tau) * e;
		}
		
	}

	double getUpdate(double prediction, double label) {
		double y = (label == 0.0) ? -1.0 : 1.0;
		double e = y - prediction;
		if(e == 0) return 0;
		if(e > 0) {
			return tau;
		} else {
			return -(1 - tau);
		}
	}

	double tau;
};

loss_function* getLossFunction(string funcName, double function_parameter) {
	if(funcName.compare("squaredloss") == 0) {
		return new squaredloss();
	} else if(funcName.compare("hingeloss") == 0) {
		return new hingeloss();
	} else if(funcName.compare("logloss") == 0) {
		return new logloss();
	} else if(funcName.compare("quantilesloss") == 0 || funcName.compare("pinballloss") == 0) {
		return new quantilesloss(function_parameter);
	} else {
		return NULL;
	}
}

