/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */

#pragma once

#include <algorithm>
#include <limits>
#include <tuple>
#include "vw_exception.h"
#include "io_buf.h"

namespace VW
{
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

    void save_load(io_buf& model_file, bool read, bool text)
    {
      if (model_file.num_files() == 0) { return; }
      std::stringstream msg;
      if (!read) { msg << "_duals_unbounded " << unbounded << "\n"; }
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&unbounded), sizeof(unbounded), "", read, msg, text);

      if (!read) { msg << "_duals_kappa " << kappa << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&kappa), sizeof(kappa), "", read, msg, text);

      if (!read) { msg << "_duals_gamma " << gamma << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&gamma), sizeof(gamma), "", read, msg, text);

      if (!read) { msg << "_duals_beta " << beta << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&beta), sizeof(beta), "", read, msg, text);

      if (!read) { msg << "_duals_n " << n << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&n), sizeof(n), "", read, msg, text);
    }

    void reset()
    {
      unbounded = false;
      kappa = 0.f;
      gamma = 0.f;
      beta = 0.f;
      n = 0.0;
    }
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

    void save_load(io_buf& model_file, bool read, bool text)
    {
      if (model_file.num_files() == 0) { return; }
      std::stringstream msg;
      if (!read) { msg << "_chisq_chi_scored " << duals.first << "\n"; }
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&duals.first), sizeof(duals.first), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_duals_stale " << duals_stale << "\n"; }
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&duals_stale), sizeof(duals_stale), "", read, msg, text);

      duals.second.save_load(model_file, read, text);

      if (!read) { msg << "_chisq_chi_alpha " << alpha << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&alpha), sizeof(alpha), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_tau " << tau << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&tau), sizeof(tau), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_wmin " << wmin << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&wmin), sizeof(wmin), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_wmax " << wmax << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&wmax), sizeof(wmax), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_rmin " << rmin << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&rmin), sizeof(rmin), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_rmax " << rmax << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&rmax), sizeof(rmax), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_n " << n << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&n), sizeof(n), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_sumw " << sumw << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&sumw), sizeof(sumw), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_sumwsq " << sumwsq << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&sumwsq), sizeof(sumwsq), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_sumwr " << sumwr << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&sumwr), sizeof(sumwr), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_sumwsqr " << sumwsqr << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&sumwsqr), sizeof(sumwsqr), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_sumwsqrsq " << sumwsqrsq << "\n"; }
      bin_text_read_write_fixed(
          model_file, reinterpret_cast<char*>(&sumwsqrsq), sizeof(sumwsqrsq), "", read, msg, text);

      if (!read) { msg << "_chisq_chi_delta " << delta << "\n"; }
      bin_text_read_write_fixed(model_file, reinterpret_cast<char*>(&delta), sizeof(delta), "", read, msg, text);
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

    ScoredDual recompute_duals();
    static double chisq_onedof_isf(double alpha);
    const double& effn() { return n; }
  };

}  // namespace distributionally_robust
}  // namespace VW
