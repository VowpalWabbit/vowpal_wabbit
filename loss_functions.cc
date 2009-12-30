/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#include "loss_functions.h"
#include<math.h>

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
		if(prediction == label) return 0;
		return label;

	}
};

class logloss : public loss_function {
public:
	logloss() {

	}

	double getLoss(double prediction, double label) {
		return log(1 + exp(-label * prediction));
	}

	double getUpdate(double prediction, double label) {
		double d = exp(-label * prediction);
		return label * d / (1 + d);
	}
};

class quantileloss : public loss_function {
public:
	quantileloss(double &tau_) : tau(tau_) {
	}

	double getLoss(double prediction, double label) {
		double e = label - prediction;
		if(e > 0) {
			return tau * e;
		} else {
			return -(1 - tau) * e;
		}
		
	}

	double getUpdate(double prediction, double label) {
		double e = label - prediction;
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
	} else if(funcName.compare("quantileloss") == 0 || funcName.compare("pinballloss") == 0) {
		return new quantileloss(function_parameter);
	} else {
		return NULL;
	}
}
