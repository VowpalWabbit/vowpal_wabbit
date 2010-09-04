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
#include "global_data.h"

class squaredloss : public loss_function {
public:
  squaredloss() {
    
  }
  
  double getLoss(double prediction, double label) {
    double example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  
  double getUpdate(double prediction, double label,double eta_t, double norm, float h) {
    eta_t*=h;
    if (eta_t<1e-12){ 
      /* When exp(-eta_t)~= 1 we replace 1-exp(-eta_t) 
       * with its first order Taylor expansion around 0
       * to avoid catastrophic cancellation.
       */
      return (label - prediction)*eta_t/norm;
    }
    return (label - prediction)*(1-exp(-eta_t))/norm;
  }
  
  //Second order update
  //double getUpdate(double prediction, double label,double eta_t, double norm, float h) {
  //	return h*eta_t*(label - prediction)/(1+h*eta_t*norm);
  //}
};

class classic_squaredloss : public loss_function {
public:
  classic_squaredloss() {
    
  }
  
  double getLoss(double prediction, double label) {
    double example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  
  double getUpdate(double prediction, double label,double eta_t, double norm, float h) {
    return h*eta_t*(label - prediction)/norm;
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
    double s1=(label*label-label*prediction)/(label*label);
    double s2=eta_t*h;
    return label * (s1<s2 ? s1 : s2)/norm;
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
    double b,l,q;
    double d = exp(label * prediction);
    double x = eta_t*h + label*prediction + d;
    /* This piece of code is approximating W(exp(x))-x. 
     * W is the Lambert W function.
     * Faster/better approximations can be substituted here 
     */
    if (x >= 1){
      l=log(x);
      q=(2.16612+1.89678*x)/(2.16276+1.90021*x-l);
      b=-x*l/(q+x);
    }
    else if(x<-7.010881832645721){
      b=-x;
    }
    else{
      b=0.566841-x*(0.637815-x*(0.0752909-x*(0.00122244+x*(0.00284082+x*(0.000413765+0.0000193232*x)))));
    }
    return -(label*b+prediction)/norm;
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
      s2=e/tau;
      return tau*(s1<s2?s1:s2)/norm;
    } else {
      s2=-e/(1-tau);
      return -(1 - tau)*(s1<s2?s1:s2)/norm;
    }
  }
  
  double tau;
};

loss_function* getLossFunction(string funcName, double function_parameter) {
  if(funcName.compare("squared") == 0) {
    return new squaredloss();
  } else if(funcName.compare("classic") == 0){
    return new classic_squaredloss();
  } else if(funcName.compare("hinge") == 0) {
    return new hingeloss();
  } else if(funcName.compare("logistic") == 0) {
    global.min_label = -100;
    global.max_label = 100;
    return new logloss();
  } else if(funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0) {
    return new quantileloss(function_parameter);
  } else {
    cout << "Invalid loss function name: \'" << funcName << "\' Bailing!" << endl;
    exit(1);
  }
  cout << "end getLossFunction" << endl;
}
