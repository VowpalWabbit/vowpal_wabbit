/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */
#include<math.h>
#include<iostream>
#include<stdlib.h>
using namespace std;

#include "loss_functions.h"

class squaredloss : public loss_function {
public:
	squaredloss() {

	}

	double getLoss(double prediction, double label) {
		double example_loss = (prediction - label) * (prediction - label);
		return example_loss;
	}

	double getUpdate(double prediction, double label,double eta_t, double norm, float h) {
		return (label - prediction)*(1-exp(-h*eta_t*norm))/norm;
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

	double getUpdate(double prediction, double label,double eta_t, double norm, float h) {
		if(label*prediction >= label*label) return 0;
		  double s1=(label*label-label*prediction)/(label*label*norm);
		  double s2=eta_t*h;
		return label*(s1<s2 ? s1 : s2);
	}
};

class logloss : public loss_function {
public:
	logloss() {

	}

	double getLoss(double prediction, double label) {
		return log(1 + exp(-label * prediction));
	}

	double getUpdate(double prediction, double label, double eta_t, double norm, float h) {
    /* There's a simpler solution for this which involves approximating W(exp(x))-x */
        double s,b,q;
        double d = exp(label * prediction);
        double c = eta_t * norm * h + label * prediction + d;
        /* In general we want s = h - (W(exp(c))-d)/(eta_t*norm) where W is 
         * the Lambert W function. The following is a good approximation:
         */
        if (c <= 1){
            q = -0.915756*c+0.763451;
            /* Safe-guard a large exponent */
            b = q > 500 ? 0.45865 : 0.45865+1/(1+exp(q));
            /* Safe-guard a large exponent */
            q = b - c > 500 ? exp(c-b) : 1/(1+exp(b-c));
            s = h-((1+exp(b-1))*q-d)/(eta_t*norm);
        }
        else{
            b = log(c);
            q = 0.997415 + 0.571902*b/(0.842524 + c);
            s = (c/(c+q)*b - label * prediction)/(eta_t*norm);
        }
        return eta_t * label * s;
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

	double getUpdate(double prediction, double label, double eta_t, double norm, float h) {
		double s2;
        double e = label - prediction;
		if(e == 0) return 0;
        double s1=eta_t*h;
		if(e > 0) {
			s2=e/(norm*tau);
			return tau*(s1<s2?s1:s2);
		} else {
			s2=-e/(norm*(1-tau));
			return -(1 - tau)*(s1<s2?s1:s2);
		}
	}

	double tau;
};

loss_function* getLossFunction(string funcName, double function_parameter) {
	if(funcName.compare("squared") == 0) {
		return new squaredloss();
	} else if(funcName.compare("hinge") == 0) {
		return new hingeloss();
	} else if(funcName.compare("logistic") == 0) {
		return new logloss();
	} else if(funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0) {
		return new quantileloss(function_parameter);
	} else {
	  cout << "Invalid loss function name: " << funcName << " Bailing!" << endl;
	  exit(1);
	}
  cout << "end getLossFunction" << endl;
}
