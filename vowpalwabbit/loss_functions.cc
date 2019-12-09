// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <math.h>
#include "correctedMath.h"
#include <iostream>
#include <stdlib.h>
#include <float.h>

#include "global_data.h"
#include "vw_exception.h"

class squaredloss : public loss_function
{
 public:
  std::string getType() { return "squared"; }

  float getLoss(shared_data* sd, float prediction, float label)
  {
    if (prediction <= sd->max_label && prediction >= sd->min_label)
    {
      float example_loss = (prediction - label) * (prediction - label);
      return example_loss;
    }
    else if (prediction < sd->min_label)
      if (label == sd->min_label)
        return 0.;
      else
        return (float)((label - sd->min_label) * (label - sd->min_label) +
            2. * (label - sd->min_label) * (sd->min_label - prediction));
    else if (label == sd->max_label)
      return 0.;
    else
      return float((sd->max_label - label) * (sd->max_label - label) +
          2. * (sd->max_label - label) * (prediction - sd->max_label));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update)
  {
    if (update_scale * pred_per_update < 1e-6)
    {
      /* When exp(-eta_t)~= 1 we replace 1-exp(-eta_t)
       * with its first order Taylor expansion around 0
       * to avoid catastrophic cancellation.
       */
      return 2.f * (label - prediction) * update_scale;
    }
    return (label - prediction) * (1.f - correctedExp(-2.f * update_scale * pred_per_update)) / pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float getRevertingWeight(shared_data* sd, float prediction, float eta_t)
  {
    float t = 0.5f * (sd->min_label + sd->max_label);
    float alternative = (prediction > t) ? sd->min_label : sd->max_label;
    return log((alternative - prediction) / (alternative - t)) / eta_t;
  }

  float getSquareGrad(float prediction, float label) { return 4.f * (prediction - label) * (prediction - label); }
  float first_derivative(shared_data* sd, float prediction, float label)
  {
    if (prediction < sd->min_label)
      prediction = sd->min_label;
    else if (prediction > sd->max_label)
      prediction = sd->max_label;
    return 2.f * (prediction - label);
  }
  float second_derivative(shared_data* sd, float prediction, float)
  {
    if (prediction <= sd->max_label && prediction >= sd->min_label)
      return 2.;
    else
      return 0.;
  }
};

class classic_squaredloss : public loss_function
{
 public:
  std::string getType() { return "classic"; }

  float getLoss(shared_data*, float prediction, float label)
  {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }

  float getUpdate(float prediction, float label, float update_scale, float /* pred_per_update */)
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float getRevertingWeight(shared_data* sd, float prediction, float eta_t)
  {
    float t = 0.5f * (sd->min_label + sd->max_label);
    float alternative = (prediction > t) ? sd->min_label : sd->max_label;
    return (t - prediction) / ((alternative - prediction) * eta_t);
  }

  float getSquareGrad(float prediction, float label) { return 4.f * (prediction - label) * (prediction - label); }
  float first_derivative(shared_data*, float prediction, float label) { return 2.f * (prediction - label); }
  float second_derivative(shared_data*, float, float) { return 2.; }
};

class hingeloss : public loss_function
{
 public:
  std::string getType() { return "hinge"; }

  float getLoss(shared_data*, float prediction, float label)
  {
    if (label != -1.f && label != 1.f)
      std::cout << "You are using label " << label << " not -1 or 1 as loss function expects!" << std::endl;
    float e = 1 - label * prediction;
    return (e > 0) ? e : 0;
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update)
  {
    if (label * prediction >= 1)
      return 0;
    float err = 1 - label * prediction;
    return label * (update_scale * pred_per_update < err ? update_scale : err / pred_per_update);
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    if (label * prediction >= 1)
      return 0;
    return label * update_scale;
  }

  float getRevertingWeight(shared_data*, float prediction, float eta_t) { return fabs(prediction) / eta_t; }

  float getSquareGrad(float prediction, float label)
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float first_derivative(shared_data*, float prediction, float label) { return (label * prediction >= 1) ? 0 : -label; }

  float second_derivative(shared_data*, float, float) { return 0.; }
};

class logloss : public loss_function
{
 public:
  std::string getType() { return "logistic"; }

  float getLoss(shared_data*, float prediction, float label)
  {
    if (label != -1.f && label != 1.f)
      std::cout << "You are using label " << label << " not -1 or 1 as loss function expects!" << std::endl;
    return log(1 + correctedExp(-label * prediction));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update)
  {
    float w, x;
    float d = correctedExp(label * prediction);
    if (update_scale * pred_per_update < 1e-6)
    {
      /* As with squared loss, for small eta_t we replace the update
       * with its first order Taylor expansion to avoid numerical problems
       */
      return label * update_scale / (1 + d);
    }
    x = update_scale * pred_per_update + label * prediction + d;
    w = wexpmx(x);
    return -(label * w + prediction) / pred_per_update;
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    float d = correctedExp(label * prediction);
    return label * update_scale / (1 + d);
  }

  inline float wexpmx(float x)
  {
    /* This piece of code is approximating W(exp(x))-x.
     * W is the Lambert W function: W(z)*exp(W(z))=z.
     * The absolute error of this approximation is less than 9e-5.
     * Faster/better approximations can be substituted here.
     */
    double w = x >= 1. ? 0.86 * x + 0.01 : correctedExp(0.8 * x - 0.65);  // initial guess
    double r = x >= 1. ? x - log(w) - w : 0.2 * x + 0.65 - w;             // residual
    double t = 1. + w;
    double u = 2. * t * (t + 2. * r / 3.);                          // magic
    return (float)(w * (1. + r / t * (u - r) / (u - 2. * r)) - x);  // more magic
  }

  float getRevertingWeight(shared_data*, float prediction, float eta_t)
  {
    float z = -fabs(prediction);
    return (1 - z - correctedExp(z)) / eta_t;
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    float v = -label / (1 + correctedExp(label * prediction));
    return v;
  }

  float getSquareGrad(float prediction, float label)
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float second_derivative(shared_data*, float prediction, float label)
  {
    float p = 1 / (1 + correctedExp(label * prediction));

    return p * (1 - p);
  }
};

class quantileloss : public loss_function
{
 public:
  quantileloss(float& tau_) : tau(tau_) {}

  std::string getType() { return "quantile"; }

  float getLoss(shared_data*, float prediction, float label)
  {
    float e = label - prediction;
    if (e > 0)
      return tau * e;
    else
      return -(1 - tau) * e;
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update)
  {
    float err = label - prediction;
    if (err == 0)
      return 0;
    float normal = update_scale * pred_per_update;  // base update size
    if (err > 0)
    {
      normal = tau * normal;
      return (normal < err ? tau * update_scale : err / pred_per_update);
    }
    else
    {
      normal = -(1 - tau) * normal;
      return (normal > err ? (tau - 1) * update_scale : err / pred_per_update);
    }
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    float err = label - prediction;
    if (err == 0)
      return 0;
    if (err > 0)
      return tau * update_scale;
    return -(1 - tau) * update_scale;
  }

  float getRevertingWeight(shared_data* sd, float prediction, float eta_t)
  {
    float v, t;
    t = 0.5f * (sd->min_label + sd->max_label);
    if (prediction > t)
      v = -(1 - tau);
    else
      v = tau;
    return (t - prediction) / (eta_t * v);
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    float e = label - prediction;
    if (e == 0)
      return 0;
    return e > 0 ? -tau : (1 - tau);
  }

  float getSquareGrad(float prediction, float label)
  {
    float fd = first_derivative(nullptr, prediction, label);
    return fd * fd;
  }

  float second_derivative(shared_data*, float, float) { return 0.; }

  float tau;
};

class poisson_loss : public loss_function
{
 public:
  std::string getType() { return "poisson"; }

  float getLoss(shared_data*, float prediction, float label)
  {
    if (label < 0.f)
      std::cout << "You are using label " << label << " but loss function expects label >= 0!" << std::endl;
    float exp_prediction = expf(prediction);
    // deviance is used instead of log-likelihood
    return 2 * (label * (logf(label + 1e-6f) - prediction) - (label - exp_prediction));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update)
  {
    float exp_prediction = expf(prediction);
    if (label > 0)
    {
      return label * update_scale -
          log1p(exp_prediction * expm1(label * update_scale * pred_per_update) / label) / pred_per_update;
    }
    else
    {
      return -log1p(exp_prediction * update_scale * pred_per_update) / pred_per_update;
    }
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale)
  {
    float exp_prediction = expf(prediction);
    return (label - exp_prediction) * update_scale;
  }

  float getRevertingWeight(shared_data* /* sd */, float /* prediction */, float /* eta_t */)
  {
    THROW("Active learning not supported by poisson loss");
  }

  float getSquareGrad(float prediction, float label)
  {
    float exp_prediction = expf(prediction);
    return (exp_prediction - label) * (exp_prediction - label);
  }

  float first_derivative(shared_data*, float prediction, float label)
  {
    float exp_prediction = expf(prediction);
    return (exp_prediction - label);
  }

  float second_derivative(shared_data*, float prediction, float /* label */)
  {
    float exp_prediction = expf(prediction);
    return exp_prediction;
  }
};

loss_function* getLossFunction(vw& all, std::string funcName, float function_parameter)
{
  if (funcName.compare("squared") == 0 || funcName.compare("Huber") == 0)
    return new squaredloss();
  else if (funcName.compare("classic") == 0)
    return new classic_squaredloss();
  else if (funcName.compare("hinge") == 0)
    return new hingeloss();
  else if (funcName.compare("logistic") == 0)
  {
    if (all.set_minmax != noop_mm)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return new logloss();
  }
  else if (funcName.compare("quantile") == 0 || funcName.compare("pinball") == 0 || funcName.compare("absolute") == 0)
  {
    return new quantileloss(function_parameter);
  }
  else if (funcName.compare("poisson") == 0)
  {
    if (all.set_minmax != noop_mm)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return new poisson_loss();
  }
  else
    THROW("Invalid loss function name: \'" << funcName << "\' Bailing!");
}
