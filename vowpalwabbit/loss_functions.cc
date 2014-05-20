/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include<math.h>
#include<iostream>
#include<stdlib.h>
#include<assert.h>
#include<float.h>
using namespace std;

#include "loss_functions.h"
#include "global_data.h"

class squaredloss : public loss_function {
public:
  squaredloss() {
    
  }
  
  float getLoss(shared_data* sd, float prediction, float label) {
    if (prediction <= sd->max_label && prediction >= sd->min_label)
      {
	float example_loss = (prediction - label) * (prediction - label);
	return example_loss;
      }
    else if (prediction < sd->min_label)
      if (label == sd->min_label)
	return 0.;
      else
	return (float) ((label - sd->min_label) * (label - sd->min_label) 
	  + 2. * (label-sd->min_label) * (sd->min_label - prediction));
    else 
      if (label == sd->max_label)
	return 0.;
      else
	return float((sd->max_label - label) * (sd->max_label - label) 
	  + 2. * (sd->max_label - label) * (prediction - sd->max_label));
  }
  
  float getUpdate(float prediction, float label, float eta_t, float pred_per_update) 
  {
    if (eta_t < 1e-6){ 
      /* When exp(-eta_t)~= 1 we replace 1-exp(-eta_t) 
       * with its first order Taylor expansion around 0
       * to avoid catastrophic cancellation.
       */
      return 2.f*(label - prediction)*eta_t/pred_per_update;
    }
    return (label - prediction)*(1.f-exp(-2.f*eta_t))/pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label, float eta_t, float pred_per_update) {
    return 2.f*(label - prediction)*eta_t/pred_per_update;
  }

  float getRevertingWeight(shared_data* sd, float prediction, float eta_t){
    float t = 0.5f*(sd->min_label+sd->max_label);
    float alternative = (prediction > t) ? sd->min_label : sd->max_label;
    return log((alternative-prediction)/(alternative-t))/eta_t;
  }
  
  float getSquareGrad(float prediction, float label) {
    return 4.f*(prediction - label) * (prediction - label);
  }
  float first_derivative(shared_data* sd, float prediction, float label)
  {
    if (prediction < sd->min_label)
      prediction = sd->min_label;
    else if (prediction > sd->max_label)
      prediction = sd->max_label;
    return 2.f * (prediction-label);
  }
  float second_derivative(shared_data* sd, float prediction, float label)
  {
    if (prediction <= sd->max_label && prediction >= sd->min_label)
      return 2.;
    else 
      return 0.;
  } 
};

class classic_squaredloss : public loss_function {
public:
  classic_squaredloss() {
    
  }
  
  float getLoss(shared_data*, float prediction, float label) {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  
  float getUpdate(float prediction, float label,float eta_t, float pred_per_update) {
    return 2.f*eta_t*(label - prediction)/pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label,float eta_t,float pred_per_update) {
    return 2.f*(label - prediction)*eta_t/pred_per_update;
  }
  
  float getRevertingWeight(shared_data* sd, float prediction, float eta_t){
    float t = 0.5f*(sd->min_label+sd->max_label);
    float alternative = (prediction > t) ? sd->min_label : sd->max_label;
    return (t-prediction)/((alternative-prediction)*eta_t);
  }

  float getSquareGrad(float prediction, float label) {
    return 4.f * (prediction - label) * (prediction - label);
  }
  float first_derivative(shared_data*, float prediction, float label)
  {
    return 2.f * (prediction-label);
  }
  float second_derivative(shared_data*, float prediction, float label)
  {
    return 2.;
  }
};


class hingeloss : public loss_function {
public:
  hingeloss() {
    
  }
  
  float getLoss(shared_data*, float prediction, float label) {
    assert(label == -1.f || label == 1.f);
    float e = 1 - label*prediction;
    return (e > 0) ? e : 0;
  }
  
  float getUpdate(float prediction, float label,float eta_t, float pred_per_update) {
    if(label*prediction >= 1) return 0;
    float err = 1 - label*prediction;
    return label * (eta_t < err ? eta_t : err)/pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label,float eta_t, float pred_per_update) {
    if(label*prediction >= 1) return 0;
    return label * eta_t/pred_per_update;
  }

  float getRevertingWeight(shared_data*, float prediction, float eta_t){
    return fabs(prediction)/eta_t;
  }

  float getSquareGrad(float prediction, float label) {
    float d = first_derivative(NULL, prediction,label);
    return d*d;
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    return (label*prediction >= 1) ? 0 : -label;
  }

  float second_derivative(shared_data*, float prediction, float label)
  {
    return 0.;
  }
};

class logloss : public loss_function {
public:
  logloss() {
    
  }
  
  float getLoss(shared_data*, float prediction, float label) {
    assert(label == -1.f || label == 1.f || label == FLT_MAX);
    return log(1 + exp(-label * prediction));
  }
  
  float getUpdate(float prediction, float label, float eta_t, float pred_per_update) {
    float w,x;
    float d = exp(label * prediction);
    if(eta_t < 1e-6){
      /* As with squared loss, for small eta_t we replace the update
       * with its first order Taylor expansion to avoid numerical problems
       */
      return label*eta_t/((1+d)*pred_per_update);
    }
    x = eta_t + label*prediction + d;
    w = wexpmx(x);
    return -(label*w+prediction)/pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label, float eta_t, float pred_per_update) {
    float d = exp(label * prediction);
    return label*eta_t/((1+d)*pred_per_update);
  }
  
  inline float wexpmx(float x){
    /* This piece of code is approximating W(exp(x))-x. 
     * W is the Lambert W function: W(z)*exp(W(z))=z.
     * The absolute error of this approximation is less than 9e-5.
     * Faster/better approximations can be substituted here.
     */
    double w = x>=1. ? 0.86*x+0.01 : exp(0.8*x-0.65); //initial guess
    double r = x>=1. ? x-log(w)-w : 0.2*x+0.65-w; //residual
    double t = 1.+w;
    double u = 2.*t*(t+2.*r/3.); //magic
    return (float)(w*(1.+r/t*(u-r)/(u-2.*r))-x); //more magic
  }
  
  float getRevertingWeight(shared_data*, float prediction, float eta_t){
    float z = -fabs(prediction);
    return (1-z-exp(z))/eta_t;
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    float v = - label/(1+exp(label * prediction));
    return v;
  }

  float getSquareGrad(float prediction, float label) {
    float d = first_derivative(NULL, prediction,label);
    return d*d;
  }

  float second_derivative(shared_data*, float prediction, float label)
  {
    float p = 1 / (1+exp(label*prediction));
    
    return p*(1-p);
  }
};

class quantileloss : public loss_function {
public:
  quantileloss(float &tau_) : tau(tau_) {
  }
  
  float getLoss(shared_data*, float prediction, float label) {
    float e = label - prediction;
    if(e > 0) {
      return tau * e;
    } else {
      return -(1 - tau) * e;
    }
    
  }
  
  float getUpdate(float prediction, float label, float eta_t, float pred_per_update) {
    float err = label - prediction;
    if(err == 0) return 0;
    float normal = eta_t;//base update size
    if(err > 0) {
      normal = tau*normal;
      return (normal < err ? normal : err) / pred_per_update;
    } else {
      normal = -(1-tau) * normal;
      return ( normal > err ?  normal : err) / pred_per_update;
    }
  }

  float getUnsafeUpdate(float prediction, float label, float eta_t, float pred_per_update) {
    float err = label - prediction;
    if(err == 0) return 0;
    if(err > 0) return tau*eta_t/pred_per_update;
    return -(1-tau)*eta_t/pred_per_update;
  }
  
  float getRevertingWeight(shared_data* sd, float prediction, float eta_t){
    float v,t;
    t = 0.5f*(sd->min_label+ sd->max_label);
    if(prediction > t)
      v = -(1-tau);
     else
      v = tau;
    return (t - prediction)/(eta_t*v);
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    float e = label - prediction; 
    if(e == 0) return 0;
    return e > 0 ? -tau : (1-tau);
  }

  float getSquareGrad(float prediction, float label) {
    float fd = first_derivative(NULL, prediction,label);
    return fd*fd;
  }

  float second_derivative(shared_data*, float prediction, float label)
  {
    return 0.;
  }

  float tau;
};

loss_function* getLossFunction(void* a, string funcName, float function_parameter) {
  vw* all=(vw*)a;
  if(funcName.compare("squared") == 0 || funcName.compare("Huber") == 0) {
    return new squaredloss();
  } else if(funcName.compare("classic") == 0){
    return new classic_squaredloss();
  } else if(funcName.compare("hinge") == 0) {
    all->sd->binary_label = true;
    return new hingeloss();
  } else if(funcName.compare("logistic") == 0) {
    if (all->set_minmax != noop_mm)
      {
	all->sd->min_label = -50;
	all->sd->max_label = 50;
	all->sd->binary_label = true;
      }
    return new logloss();
  } else if(funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0) {
    return new quantileloss(function_parameter);
  } else {
    cout << "Invalid loss function name: \'" << funcName << "\' Bailing!" << endl;
    throw exception();
  }
  cout << "end getLossFunction" << endl;
}
