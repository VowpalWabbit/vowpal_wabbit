// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimators/distributionally_robust.h"

#include "vw/core/model_utils.h"
#include "vw/core/vw_math.h"

#include <cmath>
#include <list>
#include <type_traits>

namespace VW
{
namespace estimators
{
double chi_squared::chisq_onedof_isf(double alpha)
{
  // the following is a polynomial approximation to the
  // inverse survival function for chi-squared distribution with 1 dof
  // using log and exp as basis functions
  // "constants" below found with the following Mathematica code
  //
  // data = Table[{ alpha, InverseCDF[chi_squaredistribution[1], 1 - alpha] }, { alpha, 0.001, 0.999, 0.0005 }]
  // lm = LinearModelFit[data, { Log[alpha], Log[alpha]^2, Log[alpha]^3, Log[alpha]^4 , Log[alpha]^5, Log[alpha]^6,
  // Log[alpha]^7, Log[alpha]^8, Exp[alpha], Exp[alpha]^2, Exp[alpha]^3, Exp[alpha]^4, Exp[alpha]^5, Exp[alpha]^6,
  // Exp[alpha]^7, Exp[alpha]^8}, alpha] ListPlot[lm["FitResiduals"]] lm["BestFitParameters"]
  // Show[Plot[InverseCDF[chi_squaredistribution[1], 1 - alpha], { alpha, 0 , 1}],  Plot[lm[alpha], { alpha, 0, 1 }],
  // Frame->True]

  constexpr double constants[] = {-1.73754, -1.40684, 0.0758363, 0.00726577, 0.000468688, 0.0000214395, 1.0643e-6,
      6.43011e-8, 2.0475e-9, 1.16356, -0.575446, 0.329796, -0.136076, 0.0396764, -0.00763232, 0.00087113,
      -0.0000445128};

  double logalpha = std::log(alpha);
  double expalpha = std::exp(alpha);
  double logalphapow = logalpha;
  double expalphapow = expalpha;

  double rv = constants[0];
  for (int i = 1; i <= 8; ++i)
  {
    rv += constants[i] * logalphapow;
    rv += constants[i + 8] * expalphapow;
    logalphapow *= logalpha;
    expalphapow *= expalpha;
  }

  return rv;
}

static bool isclose(double x, double y, double atol = 1e-8)
{
  double rtol = 1e-5;

  return std::abs(x - y) <= (atol + rtol * std::abs(y));
}

chi_squared::chi_squared(double alpha, double tau, double wmin, double wmax, double rmin, double _rmax)
    : _alpha(alpha)
    , _tau(tau)
    , _wmin(wmin)
    , _wmax(wmax)
    , _rmin(rmin)
    , _rmax(_rmax)
    , _n(0.0)
    , _sumw(0.0)
    , _sumwsq(0.0)
    , _sumwr(0.0)
    , _sumwsqr(0.0)
    , _sumwsqrsq(0.0)
    , _delta(chisq_onedof_isf(alpha))
    , _duals_stale(true)
{
}

bool chi_squared::is_valid() const
{
  if (_alpha > 1 || _alpha <= 0) { return false; }
  if (_tau > 1 || _tau <= 0) { return false; }
  if (_wmin >= _wmax || _wmin >= 1 || _wmax <= 1) { return false; }
  if (_rmin > _rmax) { return false; }

  return true;
}

chi_squared& chi_squared::update(double w, double r)
{
  if (w >= 0)
  {
    _n = _tau * _n + 1;
    _sumw = _tau * _sumw + w;
    _sumwsq = _tau * _sumwsq + w * w;
    _sumwr = _tau * _sumwr + w * r;
    _sumwsqr = _tau * _sumwsqr + w * w * r;
    _sumwsqrsq = _tau * _sumwsqrsq + w * w * r * r;

    _rmin = std::min(_rmin, r);
    _rmax = std::max(_rmax, r);

    _wmin = std::min(_wmin, w);
    _wmax = std::max(_wmax, w);

    _duals_stale = true;
  }

  return *this;
}

double chi_squared::qlb(double w, double r, double sign)
{
  if (_duals_stale) { recompute_duals(); }

  return _duals.second.qfunc(w, r, sign);
}

void chi_squared::reset(double alpha, double tau)
{
  _alpha = alpha;
  _tau = tau;
  _wmin = 0.0;
  _wmax = std::numeric_limits<double>::infinity();
  _rmin = 0.0;
  _rmax = 1;
  _n = 0.0;
  _sumw = 0.0;
  _sumwsq = 0.0;
  _sumwr = 0.0;
  _sumwsqr = 0.0;
  _sumwsqrsq = 0.0;
  _delta = chisq_onedof_isf(alpha);
  _duals_stale = true;
  _duals.first = 0.0;
  _duals.second.reset();
}

double chi_squared::lower_bound_and_update()
{
  if (_duals_stale) { recompute_duals(); }
  return _duals.first;
}

double chi_squared::get_phi() const
{
  double uncwfake = _sumw < _n ? _wmax : _wmin;
  double uncgstar = 0.0;

  if (uncwfake == std::numeric_limits<double>::infinity()) { uncgstar = 1.0 + 1.0 / _n; }
  else
  {
    double unca = (uncwfake + _sumw) / (1 + _n);
    double uncb = (uncwfake * uncwfake + _sumwsq) / (1 + _n);

    // NB: (uncb > unca * unca) is guaranteed
    uncgstar = (_n + 1) * (unca - 1) * (unca - 1) / (uncb - unca * unca);
  }

  return (-uncgstar - _delta) / (2 * (_n + 1));
}

VW::details::ScoredDual chi_squared::cressieread_duals(double r, double sign, double phi) const
{
  if (_n <= 0) { return std::make_pair(r, VW::details::Duals(true, 0, 0, 0, 0)); }

  std::list<VW::details::ScoredDual> candidates;
  for (auto wfake : {_wmin, _wmax})
  {
    if (wfake == std::numeric_limits<double>::infinity())
    {
      double x = sign * (r + (_sumwr - _sumw * r) / _n);
      double y = (r * _sumw - _sumwr) * (r * _sumw - _sumwr) / (_n * (1 + _n)) -
          (r * r * _sumwsq - 2 * r * _sumwsqr + _sumwsqrsq) / (1 + _n);
      double z = phi + 1 / (2 * _n);

      if (isclose(y * z, 0, 1e-9)) { y = 0; }

      if (z <= 0 && y * z >= 0)
      {
        double kappa = std::sqrt(y / (2 * z));

        if (isclose(kappa, 0))
        {
          VW::details::Duals candidate = {true, 0, 0, 0, _n};
          candidates.push_back(std::make_pair(sign * r, candidate));
        }
        else
        {
          double gstar = x - std::sqrt(2 * y * z);
          double gamma = -kappa * (1 + _n) / _n + sign * (r * _sumw - _sumwr) / _n;
          double beta = -sign * r;
          VW::details::Duals candidate = {false, kappa, gamma, beta, _n};
          candidates.push_back(std::make_pair(gstar, candidate));
        }
      }
    }
    else
    {
      double barw = (wfake + _sumw) / (1 + _n);
      double barwsq = (wfake * wfake + _sumwsq) / (1 + _n);
      double barwr = sign * (wfake * r + _sumwr) / (1 + _n);
      double barwsqr = sign * (wfake * wfake * r + _sumwsqr) / (1 + _n);
      double barwsqrsq = (wfake * wfake * r * r + _sumwsqrsq) / (1 + _n);

      if (barwsq > barw * barw)
      {
        double x = barwr + ((1 - barw) * (barwsqr - barw * barwr) / (barwsq - barw * barw));
        double y =
            (barwsqr - barw * barwr) * (barwsqr - barw * barwr) / (barwsq - barw * barw) - (barwsqrsq - barwr * barwr);
        double z = phi + 0.5 * (1 - barw) * (1 - barw) / (barwsq - barw * barw);

        if (isclose(y * z, 0, 1e-9)) { y = 0; }

        if (z <= 0 && y * z >= 0)
        {
          double kappa = std::sqrt(y / (2 * z));

          if (isclose(kappa, 0))
          {
            VW::details::Duals candidate = {true, 0, 0, 0, _n};
            candidates.push_back(std::make_pair(sign * r, candidate));
          }
          else
          {
            double gstar = x - std::sqrt(2 * y * z);
            double beta = (-kappa * (1 - barw) - (barwsqr - barw * barwr)) / (barwsq - barw * barw);
            double gamma = -kappa - beta * barw - barwr;
            VW::details::Duals candidate = {false, kappa, gamma, beta, _n};
            candidates.push_back(std::make_pair(gstar, candidate));
          }
        }
      }
    }
  }

  if (candidates.empty()) { return std::make_pair(_rmin, VW::details::Duals(true, 0, 0, 0, _n)); }
  else
  {
    auto it = std::min_element(candidates.begin(), candidates.end(),
        [](const VW::details::ScoredDual& x, const VW::details::ScoredDual& y)
        { return std::get<0>(x) < std::get<0>(y); });

    return *it;
  }
}

double chi_squared::cressieread_bound(double r, double sign, double phi) const
{
  VW::details::ScoredDual sd = cressieread_duals(r, sign, phi);
  return VW::math::clamp(sign * sd.first, _rmin, _rmax);
}

double chi_squared::cressieread_lower_bound() const { return cressieread_bound(_rmin, 1, get_phi()); }

double chi_squared::cressieread_upper_bound() const { return cressieread_bound(_rmax, -1, get_phi()); }

VW::details::ScoredDual chi_squared::recompute_duals()
{
  double r = _rmin;
  double sign = 1;
  _duals = cressieread_duals(r, sign, get_phi());
  _duals.first = VW::math::clamp(sign * _duals.first, _rmin, _rmax);
  return _duals;
}
}  // namespace estimators

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::details::Duals& _duals)
{
  size_t bytes = 0;
  bytes += read_model_field(io, _duals.unbounded);
  bytes += read_model_field(io, _duals.kappa);
  bytes += read_model_field(io, _duals.gamma);
  bytes += read_model_field(io, _duals.beta);
  bytes += read_model_field(io, _duals.n);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::details::Duals& _duals, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, _duals.unbounded, upstream_name + "_unbounded", text);
  bytes += write_model_field(io, _duals.kappa, upstream_name + "_kappa", text);
  bytes += write_model_field(io, _duals.gamma, upstream_name + "_gamma", text);
  bytes += write_model_field(io, _duals.beta, upstream_name + "_beta", text);
  bytes += write_model_field(io, _duals.n, upstream_name + "_n", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::estimators::chi_squared& chisq)
{
  size_t bytes = 0;
  bytes += read_model_field(io, chisq._alpha);
  bytes += read_model_field(io, chisq._tau);
  bytes += read_model_field(io, chisq._n);
  bytes += read_model_field(io, chisq._sumw);
  bytes += read_model_field(io, chisq._sumwsq);
  bytes += read_model_field(io, chisq._sumwr);
  bytes += read_model_field(io, chisq._sumwsqr);
  bytes += read_model_field(io, chisq._sumwsqrsq);

  bytes += read_model_field(io, chisq._rmin);
  bytes += read_model_field(io, chisq._rmax);
  bytes += read_model_field(io, chisq._wmin);
  bytes += read_model_field(io, chisq._wmax);

  chisq._duals_stale = true;
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::estimators::chi_squared& chisq, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, chisq._alpha, upstream_name + "_alpha", text);
  bytes += write_model_field(io, chisq._tau, upstream_name + "_tau", text);
  bytes += write_model_field(io, chisq._n, upstream_name + "_n", text);
  bytes += write_model_field(io, chisq._sumw, upstream_name + "_sumw", text);
  bytes += write_model_field(io, chisq._sumwsq, upstream_name + "_sumwsq", text);
  bytes += write_model_field(io, chisq._sumwr, upstream_name + "_sumwr", text);
  bytes += write_model_field(io, chisq._sumwsqr, upstream_name + "_sumwsqr", text);
  bytes += write_model_field(io, chisq._sumwsqrsq, upstream_name + "_sumwsqrsq", text);

  bytes += write_model_field(io, chisq._rmin, upstream_name + "_rmin", text);
  bytes += write_model_field(io, chisq._rmax, upstream_name + "_rmax", text);
  bytes += write_model_field(io, chisq._wmin, upstream_name + "_wmin", text);
  bytes += write_model_field(io, chisq._wmax, upstream_name + "_wmax", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
