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
  
  float getLoss(float prediction, float label) {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  
  float getUpdate(float prediction, float label,float eta_t, float norm) {
    if (eta_t < 1e-6){ 
      /* When exp(-eta_t)~= 1 we replace 1-exp(-eta_t) 
       * with its first order Taylor expansion around 0
       * to avoid catastrophic cancellation.
       */
      return (label - prediction)*eta_t/norm;
    }
    return (label - prediction)*(1-exp(-eta_t))/norm;
  }
  
  float getRevertingWeight(float prediction, float eta_t){
    float alternative = (prediction > 0.5) ? 0 : 1;
    return log((alternative-prediction)/(alternative-0.5))/eta_t;
  }
  
  float getSquareGrad(float prediction, float label) {
    return (prediction - label) * (prediction - label);
  }
  
};

class classic_squaredloss : public loss_function {
public:
  classic_squaredloss() {
    
  }
  
  float getLoss(float prediction, float label) {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  
  float getUpdate(float prediction, float label,float eta_t, float norm) {
    return eta_t*(label - prediction)/norm;
  }
  
  float getRevertingWeight(float prediction, float eta_t){
    float alternative = (prediction > 0.5) ? 0 : 1;
    return log((alternative-prediction)/(alternative-0.5))/eta_t;
  }

  float getSquareGrad(float prediction, float label) {
    return (prediction - label) * (prediction - label);
  }

};


class hingeloss : public loss_function {
public:
  hingeloss() {
    
  }
  
  float getLoss(float prediction, float label) {
    float e = 1 - label*prediction;
    return (e > 0) ? e : 0;
  }
  
  float getUpdate(float prediction, float label,float eta_t, float norm) {
    if(label*prediction >= label*label) return 0;
    float s1=(label*label-label*prediction)/(label*label);
    float s2=eta_t;
    return label * (s1<s2 ? s1 : s2)/norm;
  }
  
  float getRevertingWeight(float prediction, float eta_t){
    return fabs(prediction)/eta_t;
  }

  float getSquareGrad(float prediction, float label) {
    return (label*prediction >= label*label) ? 0 : 1;
  }
};

class logloss : public loss_function {
public:
  logloss() {
    
  }
  
  float getLoss(float prediction, float label) {
    return log(1 + exp(-label * prediction));
  }
  
  float getUpdate(float prediction, float label, float eta_t, float norm) {
    float w,x;
    float d = exp(label * prediction);
    if(eta_t < 1e-6){
      /* As with squared loss, for small eta_t we replace the update
       * with its first order Taylor expansion to avoid numerical problems
       */
      return label*eta_t/((1+d)*norm);
    }
    x = eta_t + label*prediction + d;
    w = wexpmx(x);
    return -(label*w+prediction)/norm;
  }
  
  inline float wexpmx(float x){
    float b,l,q;
    /* This piece of code is approximating W(exp(x))-x. 
     * W is the Lambert W function: W(z)*exp(W(z))=z.
     * The absolute error of this approximation is less than 9e-5.
     * Faster/better approximations can be substituted here.
     */
    if (x >= 1){
      /* This part is essentially inequality 2.4 from the paper 
       * INEQUALITIES ON THE LAMBERT W FUNCTION AND HYPERPOWER FUNCTION
       * q has been worked out so that b is an estimate rather than a bound
       */
      l=log(x);
      q=(2.16612+1.89678*x)/(2.16276+1.90021*x-l);
      b=-x*l/(q+x);
    } else if(x>-2){
      /* This comes from a Chebyshev approximation of the function in [-2,1] */
      b=0.5671307552778-x*(0.63813443251+x*(-0.073778671124427+x*(0.001712349389983*x+0.0011058485512683)));
    } else {
      /* For smaller x, we fit a logistic. Since x<0 q cannot blow up. */
      q=exp(1.00579*x +0.0186664);
      b=q/(1+q)-x;
    }
    return b;
  }
  
  float getRevertingWeight(float prediction, float eta_t){
    float z = -fabs(prediction);
    return (1-z-exp(z))/eta_t;
  }

  float getSquareGrad(float prediction, float label) {
    float d = 1./(1+exp(label * prediction));
    return d*d;
  }

};

class quantileloss : public loss_function {
public:
  quantileloss(double &tau_) : tau(tau_) {
  }
  
  float getLoss(float prediction, float label) {
    float e = label - prediction;
    if(e > 0) {
      return tau * e;
    } else {
      return -(1 - tau) * e;
    }
    
  }
  
  float getUpdate(float prediction, float label, float eta_t, float norm) {
    float s2;
    float e = label - prediction;
    if(e == 0) return 0;
    float s1=eta_t;
    if(e > 0) {
      s2=e/tau;
      return tau*(s1<s2?s1:s2)/norm;
    } else {
      s2=-e/(1-tau);
      return -(1 - tau)*(s1<s2?s1:s2)/norm;
    }
  }
  
  float getRevertingWeight(float prediction, float eta_t){
    float alternative,v;
    if(prediction > tau){
      alternative = 0;
      v = -(1-tau);
    } else{
      alternative = 1;
      v = tau;
    }
    return (alternative - prediction)/(eta_t*v);
  }

  float getSquareGrad(float prediction, float label) {
    float e = label - prediction; 
    if(e == 0) return 0;
    return e > 0 ? tau*tau : (1-tau)*(1-tau);
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
