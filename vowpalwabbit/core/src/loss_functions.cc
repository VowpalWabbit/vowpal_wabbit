// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/loss_functions.h"

#include "vw/common/vw_exception.h"
#include "vw/core/correctedMath.h"
#include "vw/core/global_data.h"
#include "vw/core/shared_data.h"
#include "vw/io/logger.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>

namespace
{
inline float squared_loss_impl_get_loss(const VW::shared_data* sd, float prediction, float label)
{
  if (prediction <= sd->max_label && prediction >= sd->min_label)
  {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }
  else if (prediction < sd->min_label)
  {
    if (label == sd->min_label) { return 0.; }
    else
    {
      return static_cast<float>((label - sd->min_label) * (label - sd->min_label) +
          2. * (label - sd->min_label) * (sd->min_label - prediction));
    }
  }
  else if (label == sd->max_label) { return 0.; }
  else
  {
    return static_cast<float>((sd->max_label - label) * (sd->max_label - label) +
        2. * (sd->max_label - label) * (prediction - sd->max_label));
  }
}

inline float squared_loss_impl_get_update(float prediction, float label, float update_scale, float pred_per_update)
{
  if (update_scale * pred_per_update < 1e-6)
  {
    /* When exp(-eta_t)~= 1 we replace 1-exp(-eta_t)
     * with its first order Taylor expansion around 0
     * to avoid catastrophic cancellation.
     */
    return 2.f * (label - prediction) * update_scale;
  }
  return (label - prediction) * (1.f - VW::details::correctedExp(-2.f * update_scale * pred_per_update)) /
      pred_per_update;
}

inline float squared_loss_impl_get_unsafe_update(float prediction, float label, float update_scale)
{
  return 2.f * (label - prediction) * update_scale;
}

inline float squared_loss_impl_get_square_grad(float prediction, float label)
{
  return 4.f * (prediction - label) * (prediction - label);
}

inline float squared_loss_impl_first_derivative(const VW::shared_data* sd, float prediction, float label)
{
  if (prediction < sd->min_label) { prediction = sd->min_label; }
  else if (prediction > sd->max_label) { prediction = sd->max_label; }
  return 2.f * (prediction - label);
}

inline float squared_loss_impl_second_derivative(const VW::shared_data* sd, float prediction)
{
  if (prediction <= sd->max_label && prediction >= sd->min_label) { return 2.; }
  else { return 0.; }
}

class squaredloss : public VW::loss_function
{
public:
  std::string get_type() const override { return "squared"; }

  float get_loss(const VW::shared_data* sd, float prediction, float label) const override
  {
    return squared_loss_impl_get_loss(sd, prediction, label);
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    return squared_loss_impl_get_update(prediction, label, update_scale, pred_per_update);
  }

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    return squared_loss_impl_get_unsafe_update(prediction, label, update_scale);
  }

  float get_square_grad(float prediction, float label) const override
  {
    return squared_loss_impl_get_square_grad(prediction, label);
  }

  float first_derivative(const VW::shared_data* sd, float prediction, float label) const override
  {
    return squared_loss_impl_first_derivative(sd, prediction, label);
  }

  float second_derivative(const VW::shared_data* sd, float prediction, float) const override
  {
    return squared_loss_impl_second_derivative(sd, prediction);
  }
};

class classic_squaredloss : public VW::loss_function
{
public:
  std::string get_type() const override { return "classic"; }

  float get_loss(const VW::shared_data*, float prediction, float label) const override
  {
    float example_loss = (prediction - label) * (prediction - label);
    return example_loss;
  }

  float get_update(float prediction, float label, float update_scale, float /* pred_per_update */) const override
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    return 2.f * (label - prediction) * update_scale;
  }

  float get_square_grad(float prediction, float label) const override
  {
    return 4.f * (prediction - label) * (prediction - label);
  }

  float first_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    return 2.f * (prediction - label);
  }

  float second_derivative(const VW::shared_data*, float, float) const override { return 2.; }
};

class hingeloss : public VW::loss_function
{
public:
  explicit hingeloss(VW::io::logger logger) : _logger(std::move(logger)) {}

  std::string get_type() const override { return "hinge"; }

  float get_loss(const VW::shared_data*, float prediction, float label) const override
  {
    if (label != -1.f && label != 1.f)
    {
      _logger.out_warn("The label {} is not -1 or 1 or in [0,1] as the hinge loss function expects.", label);
    }
    float e = 1 - label * prediction;
    return (e > 0) ? e : 0;
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    if (label * prediction >= 1) { return 0; }
    float err = 1 - label * prediction;
    return label * (update_scale * pred_per_update < err ? update_scale : err / pred_per_update);
  }

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    if (label * prediction >= 1) { return 0; }
    return label * update_scale;
  }

  float get_square_grad(float prediction, float label) const override
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float first_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    return (label * prediction >= 1) ? 0 : -label;
  }

  float second_derivative(const VW::shared_data*, float, float) const override { return 0.; }

private:
  mutable VW::io::logger _logger;
};

class logloss : public VW::loss_function
{
public:
  explicit logloss(VW::io::logger logger, float loss_min, float loss_max)
      : _logger(std::move(logger)), _loss_min(loss_min), _loss_max(loss_max)
  {
  }

  std::string get_type() const override { return "logistic"; }

  float get_loss(const VW::shared_data*, float prediction, float label) const override
  {
    if (label < _loss_min || label > _loss_max)
    {
      _logger.out_warn("The label {} is not in the range [{},{}] as the logistic loss function expects.", label,
          _loss_min, _loss_max);
    }
    float std_label = (label - _loss_min) / (_loss_max - _loss_min);
    return std_label * get_loss_sub(prediction, 1.f) + (1 - std_label) * get_loss_sub(prediction, -1.f);
  }

  float get_loss_sub(float prediction, float label) const
  {
    if (label != -1.f && label != 1.f)
    {
      _logger.out_warn("The label {} is not -1 or 1 after rounding as the logistic loss function expects.", label);
    }
    return std::log(1 + VW::details::correctedExp(-label * prediction));
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    float std_label = (label - _loss_min) / (_loss_max - _loss_min);
    return std_label * get_update_sub(prediction, 1.f, update_scale, pred_per_update) +
        (1 - std_label) * get_update_sub(prediction, -1.f, update_scale, pred_per_update);
  }

  float get_update_sub(float prediction, float label, float update_scale, float pred_per_update) const
  {
    float w, x;
    float d = VW::details::correctedExp(label * prediction);
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

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    float std_label = (label - _loss_min) / (_loss_max - _loss_min);
    return std_label * get_unsafe_update_sub(prediction, 1.f, update_scale) +
        (1 - std_label) * get_unsafe_update_sub(prediction, -1.f, update_scale);
  }

  float get_unsafe_update_sub(float prediction, float label, float update_scale) const
  {
    float d = VW::details::correctedExp(label * prediction);
    return label * update_scale / (1 + d);
  }

  static inline float wexpmx(float x)
  {
    /* This piece of code is approximating W(exp(x))-x.
     * W is the Lambert W function: W(z)*exp(W(z))=z.
     * The absolute error of this approximation is less than 9e-5.
     * Faster/better approximations can be substituted here.
     */
    double w = x >= 1. ? 0.86 * x + 0.01 : VW::details::correctedExp(0.8 * x - 0.65);  // initial guess
    double r = x >= 1. ? x - log(w) - w : 0.2 * x + 0.65 - w;                          // residual
    double t = 1. + w;
    double u = 2. * t * (t + 2. * r / 3.);                                     // magic
    return static_cast<float>(w * (1. + r / t * (u - r) / (u - 2. * r)) - x);  // more magic
  }

  float first_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    float std_label = (label - _loss_min) / (_loss_max - _loss_min);
    return std_label * first_derivative_sub(prediction, 1.f) + (1 - std_label) * first_derivative_sub(prediction, -1.f);
  }

  float first_derivative_sub(float prediction, float label) const
  {
    float v = -label / (1 + VW::details::correctedExp(label * prediction));
    return v;
  }

  float get_square_grad(float prediction, float label) const override
  {
    float d = first_derivative(nullptr, prediction, label);
    return d * d;
  }

  float second_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    float std_label = (label - _loss_min) / (_loss_max - _loss_min);
    return std_label * second_derivative_sub(prediction, 1.f) +
        (1 - std_label) * second_derivative_sub(prediction, -1.f);
  }

  float second_derivative_sub(float prediction, float label) const
  {
    float p = 1 / (1 + VW::details::correctedExp(label * prediction));

    return p * (1 - p);
  }

private:
  mutable VW::io::logger _logger;
  float _loss_min;
  float _loss_max;
};

class quantileloss : public VW::loss_function
{
public:
  quantileloss(float& tau_) : tau(tau_) {}

  std::string get_type() const override { return "quantile"; }
  float get_parameter() const override { return tau; }

  float get_loss(const VW::shared_data*, float prediction, float label) const override
  {
    float e = label - prediction;
    if (e > 0) { return tau * e; }
    else { return -(1 - tau) * e; }
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    float err = label - prediction;
    if (err == 0) { return 0; }
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

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    float err = label - prediction;
    if (err == 0) { return 0; }
    if (err > 0) { return tau * update_scale; }
    return -(1 - tau) * update_scale;
  }

  float first_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    float e = label - prediction;
    if (e == 0) { return 0; }
    return e > 0 ? -tau : (1 - tau);
  }

  float get_square_grad(float prediction, float label) const override
  {
    float fd = first_derivative(nullptr, prediction, label);
    return fd * fd;
  }

  float second_derivative(const VW::shared_data*, float, float) const override { return 0.; }

  float tau;
};

// Expectile loss is closely related to the squared loss, but it's an asymmetric function with an expectile parameter
// Its methods can be derived from the corresponding methods from the squared loss multiplied by the expectile value
class expectileloss : public VW::loss_function
{
public:
  expectileloss(float& q) : _q(q) {}

  std::string get_type() const override { return "expectile"; }
  float get_parameter() const override { return _q; }

  float get_loss(const VW::shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squared_loss_impl_get_loss(sd, prediction, label) * (err < 0 ? _q : (1.f - _q));
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    float err = label - prediction;
    return err < 0 ? squared_loss_impl_get_update(prediction, label, _q * update_scale, pred_per_update)
                   : squared_loss_impl_get_update(prediction, label, (1.f - _q) * update_scale, pred_per_update);
  }

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    float err = label - prediction;
    return err < 0 ? squared_loss_impl_get_unsafe_update(prediction, label, _q * update_scale)
                   : squared_loss_impl_get_unsafe_update(prediction, label, (1.f - _q) * update_scale);
  }

  float get_square_grad(float prediction, float label) const override
  {
    float err = label - prediction;
    return squared_loss_impl_get_square_grad(prediction, label) * (err < 0 ? _q * _q : (1.f - _q) * (1.f - _q));
  }

  float first_derivative(const VW::shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squared_loss_impl_first_derivative(sd, prediction, label) * (err < 0 ? _q : (1.f - _q));
  }

  float second_derivative(const VW::shared_data* sd, float prediction, float label) const override
  {
    float err = label - prediction;
    return squared_loss_impl_second_derivative(sd, prediction) * (err < 0 ? _q : (1.f - _q));
  }

private:
  // Expectile parameter that transforms the squared loss into an asymmetric loss function if _q != 0.5
  float _q;
};

class poisson_loss : public VW::loss_function
{
public:
  explicit poisson_loss(VW::io::logger logger) : _logger(std::move(logger)) {}

  std::string get_type() const override { return "poisson"; }

  float get_loss(const VW::shared_data*, float prediction, float label) const override
  {
    if (label < 0.f) { _logger.out_warn("The poisson loss function expects a label >= 0 but received '{}'.", label); }
    float exp_prediction = std::exp(prediction);
    // deviance is used instead of log-likelihood
    return 2 * (label * (std::log(label + 1e-6f) - prediction) - (label - exp_prediction));
  }

  float get_update(float prediction, float label, float update_scale, float pred_per_update) const override
  {
    float exp_prediction = std::exp(prediction);
    if (label > 0)
    {
      return label * update_scale -
          std::log1p(exp_prediction * std::expm1(label * update_scale * pred_per_update) / label) / pred_per_update;
    }
    else { return -std::log1p(exp_prediction * update_scale * pred_per_update) / pred_per_update; }
  }

  float get_unsafe_update(float prediction, float label, float update_scale) const override
  {
    float exp_prediction = std::exp(prediction);
    return (label - exp_prediction) * update_scale;
  }

  float get_square_grad(float prediction, float label) const override
  {
    float exp_prediction = std::exp(prediction);
    return (exp_prediction - label) * (exp_prediction - label);
  }

  float first_derivative(const VW::shared_data*, float prediction, float label) const override
  {
    float exp_prediction = std::exp(prediction);
    return (exp_prediction - label);
  }

  float second_derivative(const VW::shared_data*, float prediction, float /* label */) const override
  {
    float exp_prediction = std::exp(prediction);
    return exp_prediction;
  }

private:
  mutable VW::io::logger _logger;
};
}  // namespace
namespace VW
{
std::unique_ptr<loss_function> get_loss_function(
    VW::workspace& all, const std::string& funcName, float function_parameter_0, float function_parameter_1)
{
  if (funcName == "squared" || funcName == "Huber") { return VW::make_unique<squaredloss>(); }
  else if (funcName == "classic") { return VW::make_unique<classic_squaredloss>(); }
  else if (funcName == "hinge") { return VW::make_unique<hingeloss>(all.logger); }
  else if (funcName == "logistic")
  {
    if (all.set_minmax)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return VW::make_unique<logloss>(all.logger, function_parameter_0, function_parameter_1);
  }
  else if (funcName == "quantile" || funcName == "pinball" || funcName == "absolute")
  {
    return VW::make_unique<quantileloss>(function_parameter_0);
  }
  else if (funcName == "expectile") { return VW::make_unique<expectileloss>(function_parameter_0); }
  else if (funcName == "poisson")
  {
    if (all.set_minmax)
    {
      all.sd->min_label = -50;
      all.sd->max_label = 50;
    }
    return VW::make_unique<poisson_loss>(all.logger);
  }
  else
    THROW("Invalid loss function name: \'" << funcName << "\'.");
}

}  // namespace VW
