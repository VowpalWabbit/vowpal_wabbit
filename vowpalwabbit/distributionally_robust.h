/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <algorithm>
#include <limits>
#include "vw_exception.h"

namespace VW
{
namespace distributionally_robust
{

class ChiSquared
{
  public:
    struct Duals
    {
      bool unbounded;
      double kappa;
      double gamma;
      double beta;
      double n;

      double qfunc(double w, double r) { return unbounded ? 1 : -(gamma + (beta + r) * w) / ((n + 1) * kappa); }
    };

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
    Duals duals;

  public:
    // alpha: confidence level
    // tau: count decay time constant
    explicit ChiSquared(double _alpha, 
               double _tau, 
               double _wmin = 0, 
               double _wmax = std::numeric_limits<double>::infinity(),
               double _rmin = 0,
               double _rmax = 1)
      : alpha(_alpha),
        tau(_tau),
        wmin(_wmin),
        wmax(_wmax),
        rmin(_rmin),
        rmax(_rmax),
        n(0),
        sumw(0),
        sumwsq(0),
        sumwr(0),
        sumwsqr(0),
        sumwsqrsq(0),
        delta(chisq_onedof_isf(alpha)),
        duals_stale(true)
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
            sumwsq = tau * sumwsq + w*w;
            sumwr = tau * sumwr + w*r;
            sumwsqr = tau * sumwsqr + w*w*r;
            sumwsqrsq = tau * sumwsqrsq + w*w*r*r;

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
        if (duals_stale)
          {
            recompute_duals();
          }

        return duals.qfunc(w, r);
      }

    Duals recompute_duals();
    static double chisq_onedof_isf(double alpha);
    const double& effn() { return n; }
};

}
}
