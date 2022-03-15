// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "correctedMath.h"
#include "global_data.h"
#include "io/logger.h"
#include "shared_data.h"
#include "vw_exception.h"
float squaredloss_getLoss(const shared_data* sd, float prediction, float label);
float squaredloss_getUpdate(float prediction, float label, float update_scale, float pred_per_update);
float squaredloss_getUnsafeUpdate(float prediction, float label, float update_scale);
float squaredloss_getSquareGrad(float prediction, float label);
float squaredloss_first_derivative(const shared_data* sd, float prediction, float label);
float squaredloss_second_derivative(const shared_data* sd, float prediction);

class squaredloss : public loss_function
{
public:
  std::string getType() const override { return "squared"; }

  float getLoss(const shared_data* sd, float prediction, float label) const override
  {
    return squaredloss_getLoss(sd, prediction, label);
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    return squaredloss_getUpdate(prediction, label, update_scale, pred_per_update);
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    return squaredloss_getUnsafeUpdate(prediction, label, update_scale);
  }

  float getSquareGrad(float prediction, float label) const override
  {
    return squaredloss_getSquareGrad(prediction, label);
  }

  float first_derivative(const shared_data* sd, float prediction, float label) const override
  {
    return squaredloss_first_derivative(sd, prediction, label);
  }

  float second_derivative(const shared_data* sd, float prediction, float) const override
  {
    return squaredloss_second_derivative(sd, prediction);
  }
};

class classic_squaredloss : public loss_function
{
public:
  std::string getType() const override { return "classic"; }

  float getLoss(const shared_data*, float prediction, float label) const override
  {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }

  float getUpdate(float prediction, float label, float update_scale, float /* pred_per_update */) const override
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float getSquareGrad(float prediction, float label) const override
  {
    return 4.f * (prediction - label) * (prediction - label);
  }

  float first_derivative(const shared_data*, float prediction, float label) const override
  {
    return 2.f * (prediction - label);
  }

  float second_derivative(const shared_data*, float, float) const override { return 2.; }
};

class hingeloss : public loss_function
{
  mutable VW::io::logger logger;

public:
  explicit hingeloss(VW::io::logger logger) : logger(std::move(logger)) {}

  std::string getType() const override { return "hinge"; }

  float getLoss(const shared_data*, float prediction, float label) const override
  {
    if (label != -1.f && label != 1.f)
      logger.out_warn("The label {} is not -1 or 1 or in [0,1] as the hinge loss function expects.", label);
    float e = 1 - label * prediction;
    return (e > 0) ? e : 0;
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    if (label * prediction >= 1) return 0;
    float err = 1 - label * prediction;
    return label * (update_scale * pred_per_update < err ? update_scale : err / pred_per_update);
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    if (label * prediction >= 1) return 0;
    return label * update_scale;
  }

  float getSquareGrad(float prediction, float label) const override
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float first_derivative(const shared_data*, float prediction, float label) const override
  {
    return (label * prediction >= 1) ? 0 : -label;
  }

  float second_derivative(const shared_data*, float, float) const override { return 0.; }
};

class logloss : public loss_function
{
  mutable VW::io::logger logger;

public:
  explicit logloss(VW::io::logger logger) : logger(std::move(logger)) {}

  std::string getType() const override { return "logistic"; }

  float getLoss(const shared_data*, float prediction, float label) const override
  {
    if (label != -1.f && label != 1.f)
      logger.out_warn("The label {} is not -1 or 1 or in [0,1] as the logistic loss function expects.", label);
    return log(1 + correctedExp(-label * prediction));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
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

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    float d = correctedExp(label * prediction);
    return label * update_scale / (1 + d);
  }

  static inline float wexpmx(float x)
  {
    /* This piece of code is approximating W(exp(x))-x.
     * W is the Lambert W function: W(z)*exp(W(z))=z.
     * The absolute error of this approximation is less than 9e-5.
     * Faster/better approximations can be substituted here.
     */
    double w = x >= 1. ? 0.86 * x + 0.01 : correctedExp(0.8 * x - 0.65);  // initial guess
    double r = x >= 1. ? x - log(w) - w : 0.2 * x + 0.65 - w;             // residual
    double t = 1. + w;
    double u = 2. * t * (t + 2. * r / 3.);                                     // magic
    return static_cast<float>(w * (1. + r / t * (u - r) / (u - 2. * r)) - x);  // more magic
  }

  float first_derivative(const shared_data*, float prediction, float label) const override
  {
    float v = -label / (1 + correctedExp(label * prediction));
    return v;
  }

  float getSquareGrad(float prediction, float label) const override
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float second_derivative(const shared_data*, float prediction, float label) const override
  {
    float p = 1 / (1 + correctedExp(label * prediction));

    return p * (1 - p);
  }
};

class quantileloss : public loss_function
{
public:
  quantileloss(float& tau_) : tau(tau_) {}

  std::string getType() const override { return "quantile"; }
  float getParameter() const override { return tau; }

  float getLoss(const shared_data*, float prediction, float label) const override
  {
    float e = label - prediction;
    if (e > 0)
      return tau * e;
    else
      return -(1 - tau) * e;
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    float err = label - prediction;
    if (err == 0) return 0;
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

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    float err = label - prediction;
    if (err == 0) return 0;
    if (err > 0) return tau * update_scale;
    return -(1 - tau) * update_scale;
  }

  float first_derivative(const shared_data*, float prediction, float label) const override
  {
    float e = label - prediction;
    if (e == 0) return 0;
    return e > 0 ? -tau : (1 - tau);
  }

  float getSquareGrad(float prediction, float label) const override
  {
    float fd = first_derivative(nullptr, prediction, label);
    return fd * fd;
  }

  float second_derivative(const shared_data*, float, float) const override { return 0.; }

  float tau;
};

class expectileloss : public loss_function
{
public:
  expectileloss(float& q_) : q(q_) {}

  std::string getType() const override { return "expectile"; }
  float getParameter() const override { return q; }

  // Expectile loss is closely related to the squared loss, but it's an assymetric function with a quantile parameter
  // Its methods can be derived from the corresponding methods from the squared loss multiplied by the quantile value
  float getLoss(const shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squaredloss_getLoss(sd, prediction, label) * (err > 0 ? q : (1.f - q));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    // Based on the invariant update calculation from the [Online Importance Weight Aware Updates paper](https://arxiv.org/abs/1011.1576)
    float err = label - prediction;
    if (err > 0) return squaredloss_getUpdate(prediction, label, q * update_scale, pred_per_update);
    return squaredloss_getUpdate(prediction, label, (1.f - q) * update_scale, pred_per_update);
  }

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    float err = label - prediction;
    if (err > 0) return squaredloss_getUnsafeUpdate(prediction, label, q * update_scale);
    return squaredloss_getUnsafeUpdate(prediction, label, (1.f - q) * update_scale);
  }

  float first_derivative(const shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squaredloss_first_derivative(sd, prediction, label) * (err > 0 ? q : (1.f - q));
  }

  float getSquareGrad(float prediction, float label) const override
  {
    float err = label - prediction;
    return squaredloss_getSquareGrad(prediction, label) * (err > 0 ? q * q : (1.f - q) * (1.f - q));
  }

  float second_derivative(const shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squaredloss_second_derivative(sd, prediction) * (err > 0 ? q : (1.f - q));
  }

  float q;
};

class poisson_loss : public loss_function
{
  mutable VW::io::logger logger;

public:
  explicit poisson_loss(VW::io::logger logger) : logger(std::move(logger)) {}

  std::string getType() const override { return "poisson"; }

  float getLoss(const shared_data*, float prediction, float label) const override
  {
    if (label < 0.f) { logger.out_warn("The poisson loss function expects a label >= 0 but received '{}'.", label); }
    float exp_prediction = expf(prediction);
    // deviance is used instead of log-likelihood
    return 2 * (label * (logf(label + 1e-6f) - prediction) - (label - exp_prediction));
  }

  float getUpdate(float prediction, float label, float update_scale, float pred_per_update) const override
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

  float getUnsafeUpdate(float prediction, float label, float update_scale) const override
  {
    float exp_prediction = expf(prediction);
    return (label - exp_prediction) * update_scale;
  }

  float getSquareGrad(float prediction, float label) const override
  {
    float exp_prediction = expf(prediction);
    return (exp_prediction - label) * (exp_prediction - label);
  }

  float first_derivative(const shared_data*, float prediction, float label) const override
  {
    float exp_prediction = expf(prediction);
    return (exp_prediction - label);
  }

  float second_derivative(const shared_data*, float prediction, float /* label */) const override
  {
    float exp_prediction = expf(prediction);
    return exp_prediction;
  }
};

std::unique_ptr<loss_function> getLossFunction(
    VW::workspace& all, const std::string& funcName, float function_parameter)
{
  if (funcName == "squared" || funcName == "Huber") { return VW::make_unique<squaredloss>(); }
  else if (funcName == "classic")
  {
    return VW::make_unique<classic_squaredloss>();
  }
  else if (funcName == "hinge")
  {
    return VW::make_unique<hingeloss>(all.logger);
  }
  else if (funcName == "logistic")
  {
    if (all.set_minmax != noop_mm)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return VW::make_unique<logloss>(all.logger);
  }
  else if (funcName == "quantile" || funcName == "pinball" || funcName == "absolute")
  {
    return VW::make_unique<quantileloss>(function_parameter);
  }
  else if (funcName == "expectile")
  {
    return VW::make_unique<expectileloss>(function_parameter);
  }
  else if (funcName == "poisson")
  {
    if (all.set_minmax != noop_mm)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return VW::make_unique<poisson_loss>(all.logger);
  }
  else
    THROW("Invalid loss function name: \'" << funcName << "\'.");
}

inline float squaredloss_getLoss(const shared_data* sd, float prediction, float label)
{
  if (prediction <= sd->max_label && prediction >= sd->min_label)
  {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  else if (prediction < sd->min_label)
  {
    if (label == sd->min_label)
      return 0.;
    else
      return static_cast<float>((label - sd->min_label) * (label - sd->min_label) +
          2. * (label - sd->min_label) * (sd->min_label - prediction));
  }
  else if (label == sd->max_label)
    return 0.;
  else
    return static_cast<float>((sd->max_label - label) * (sd->max_label - label) +
        2. * (sd->max_label - label) * (prediction - sd->max_label));
}

inline float squaredloss_getUpdate(float prediction, float label, float update_scale, float pred_per_update)
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

inline float squaredloss_getUnsafeUpdate(float prediction, float label, float update_scale)
{
  return 2.f * (label - prediction) * update_scale;
}

inline float squaredloss_getSquareGrad(float prediction, float label)
{
  return 4.f * (prediction - label) * (prediction - label);
}

inline float squaredloss_first_derivative(const shared_data* sd, float prediction, float label)
{
  if (prediction < sd->min_label)
    prediction = sd->min_label;
  else if (prediction > sd->max_label)
    prediction = sd->max_label;
  return 2.f * (prediction - label);
}

inline float squaredloss_second_derivative(const shared_data* sd, float prediction)
{
  if (prediction <= sd->max_label && prediction >= sd->min_label)
    return 2.;
  else
    return 0.;
}
