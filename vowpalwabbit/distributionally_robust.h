/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <algorithm>
#include <limits>
#include <tuple>
#include "io_buf.h"
#include "model_utils.h"

namespace VW
{
namespace distributionally_robust
{
struct Duals;
class ChiSquared;
}  // namespace distributionally_robust

namespace model_utils
{
size_t process_model_field(io_buf&, VW::distributionally_robust::Duals&, bool, const std::string&, bool);
size_t process_model_field(io_buf&, VW::distributionally_robust::ChiSquared&, bool, const std::string&, bool);
}  // namespace model_utils

namespace distributionally_robust
{
  struct Duals
  {
    bool unbounded;
    double kappa;
    double gamma;
    double beta;
    double n;

    Duals() : unbounded(false), kappa(0.f), gamma(0.f), beta(0.f), n(0.f) {}
    Duals(bool unbounded, double kappa, double gamma, double beta, double n)
        : unbounded(unbounded), kappa(kappa), gamma(gamma), beta(beta), n(n)
    {
    }
    double qfunc(double w, double r) { return unbounded ? 1 : -(gamma + (beta + r) * w) / ((n + 1) * kappa); }

    void reset()
    {
      unbounded = false;
      kappa = 0.f;
      gamma = 0.f;
      beta = 0.f;
      n = 0.0;
    }

    friend size_t VW::model_utils::process_model_field(
        io_buf&, VW::distributionally_robust::Duals&, bool, const std::string&, bool);
  };

  using ScoredDual = std::pair<double, Duals>;

  // https://en.wikipedia.org/wiki/Divergence_(statistics)
  class ChiSquared
  {
  private:
    double alpha;
    double tau;
    double wmin;
    double wmax;
    double rmin;
    double rmax;

    double n;
    double sumw;
    double sumwsq;
    double sumwr;
    double sumwsqr;
    double sumwsqrsq;

    double delta;

    bool duals_stale;
    ScoredDual duals;

  public:
    // alpha: confidence level
    // tau: count decay time constant
    explicit ChiSquared(double _alpha, double _tau, double _wmin = 0,
        double _wmax = std::numeric_limits<double>::infinity(), double _rmin = 0, double _rmax = 1)
        : alpha(_alpha)
        , tau(_tau)
        , wmin(_wmin)
        , wmax(_wmax)
        , rmin(_rmin)
        , rmax(_rmax)
        , n(0)
        , sumw(0)
        , sumwsq(0)
        , sumwr(0)
        , sumwsqr(0)
        , sumwsqrsq(0)
        , delta(chisq_onedof_isf(alpha))
        , duals_stale(true)
    {
    }

    bool isValid()
    {
      if (alpha > 1 || alpha <= 0) return false;
      if (tau > 1 || tau <= 0) return false;
      if (wmin >= wmax || wmin >= 1 || wmax <= 1) return false;
      if (rmin > rmax) return false;

      return true;
    }

    ChiSquared& update(double w, double r)
    {
      if (w >= 0)
      {
        n = tau * n + 1;
        sumw = tau * sumw + w;
        sumwsq = tau * sumwsq + w * w;
        sumwr = tau * sumwr + w * r;
        sumwsqr = tau * sumwsqr + w * w * r;
        sumwsqrsq = tau * sumwsqrsq + w * w * r * r;

        rmin = std::min(rmin, r);
        rmax = std::max(rmax, r);

        wmin = std::min(wmin, w);
        wmax = std::max(wmax, w);

        duals_stale = true;
      }

      return *this;
    }

    double qlb(double w, double r)
    {
      if (duals_stale) { recompute_duals(); }

      return duals.second.qfunc(w, r);
    }

    void reset(double _alpha, double _tau)
    {
      alpha = _alpha;
      tau = _tau;
      wmin = 0.0;
      wmax = std::numeric_limits<double>::infinity();
      rmin = 0.0;
      rmax = 1;
      n = 0.0;
      sumw = 0.0;
      sumwsq = 0.0;
      sumwr = 0.0;
      sumwsqr = 0.0;
      sumwsqrsq = 0.0;
      delta = chisq_onedof_isf(alpha);
      duals_stale = true;
      duals.first = 0.0;
      duals.second.reset();
    }

    double lower_bound()
    {
      if (duals_stale) { recompute_duals(); }
      return duals.first;
    }

    ScoredDual recompute_duals();
    static double chisq_onedof_isf(double alpha);
    const double& effn() { return n; }
    friend size_t VW::model_utils::process_model_field(
        io_buf&, VW::distributionally_robust::ChiSquared&, bool, const std::string&, bool);
    void save_load(io_buf& model_file, bool read, bool text, const char* name);
  };

}  // namespace distributionally_robust
}  // namespace VW
