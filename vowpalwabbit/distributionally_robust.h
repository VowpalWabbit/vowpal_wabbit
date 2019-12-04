/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace VW
{
namespace distributionally_robust
{

class ChiSquared
{
  private:
    struct Duals
    {
      bool stale;
      bool unbounded;
      double kappa;
      double gamma;
      double beta;
      double n;

      double qfunc(double w, double r)
        {
          return unbounded ? 1 : -(gamma + (beta + r) * w) / ((n + 1) * kappa);
        }
    };
    
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

    Duals duals;

  public:
    // alpha: confidence level
    // tau: count decay time constant
    ChiSquared(double _alpha, 
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
        delta(chisq_onedof_isf(alpha))
      {
        if (alpha > 1 || alpha <= 0)
          {
            throw std::invalid_argument("invalid alpha value");
          }

        if (tau > 1 || tau <= 0)
          {
            throw std::invalid_argument("invalid tau value");
          }

        if (wmin >= wmax || wmin >= 1 || wmax <= 1)
          {
            throw std::invalid_argument("invalid limits on w");
          }

        if (rmin > rmax)
          {
            throw std::invalid_argument("invalid limits on r");
          }

        duals.stale = true;
      }

    ChiSquared& update(double w, double r)
      {
        n = tau * n + 1;
        sumw = tau * sumw + w;
        sumwsq = tau * sumwsq + w*w;
        sumwr = tau * sumwr + w*r;
        sumwsqr = tau * sumwsqr + w*w*r;
        sumwsqrsq = tau * sumwsqrsq + w*w*r*r;

        rmin = std::min(rmin, r);
        rmax = std::max(rmax, r);

        duals.stale = true;

        return *this;
      }

    double qlb(double w, double r)
      {
        if (duals.stale)
          {
            recompute_duals();
          }

        return duals.qfunc(w, r);
      }

    void recompute_duals();
    static double chisq_onedof_isf(double alpha);
};

}
}
