// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <algorithm>
#include <limits>
#include <string>
#include <tuple>

constexpr float BASELINE_DEFAULT_TAU = 0.999f;
constexpr float CRESSEREAD_DEFAULT_TAU = 1.0f;
constexpr float DEFAULT_ALPHA = 0.05f;

class io_buf;
namespace VW
{
namespace distributionally_robust
{
struct Duals;
class ChiSquared;
}  // namespace distributionally_robust

namespace model_utils
{
size_t read_model_field(io_buf&, VW::distributionally_robust::Duals&);
size_t write_model_field(io_buf&, const VW::distributionally_robust::Duals&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::distributionally_robust::ChiSquared&);
size_t write_model_field(io_buf&, const VW::distributionally_robust::ChiSquared&, const std::string&, bool);
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
  double qfunc(double w, double r, double sign)
  {
    return unbounded ? 1 : -(gamma + (beta + sign * r) * w) / ((n + 1) * kappa);
  }

  void reset()
  {
    unbounded = false;
    kappa = 0.f;
    gamma = 0.f;
    beta = 0.f;
    n = 0.0;
  }

  friend size_t VW::model_utils::read_model_field(io_buf&, VW::distributionally_robust::Duals&);
  friend size_t VW::model_utils::write_model_field(
      io_buf&, const VW::distributionally_robust::Duals&, const std::string&, bool);
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
  explicit ChiSquared(double _alpha, double _tau, double _wmin = 0.0,
      double _wmax = std::numeric_limits<double>::infinity(), double _rmin = 0.0, double _rmax = 1.0);
  bool isValid() const;
  ChiSquared& update(double w, double r);
  double qlb(double w, double r, double sign);  // sign = 1.0 for lower_bound, sign = -1.0 for upper_bound
  void reset(double _alpha, double _tau);
  double lower_bound_and_update();
  double get_phi() const;
  ScoredDual cressieread_duals(double r, double sign, double phi) const;
  double cressieread_bound(double r, double sign, double phi) const;
  double cressieread_lower_bound() const;
  double cressieread_upper_bound() const;
  ScoredDual recompute_duals();
  static double chisq_onedof_isf(double alpha);
  const double& effn() { return n; }
  friend size_t VW::model_utils::read_model_field(io_buf&, VW::distributionally_robust::ChiSquared&);
  friend size_t VW::model_utils::write_model_field(
      io_buf&, const VW::distributionally_robust::ChiSquared&, const std::string&, bool);
  void save_load(io_buf& model_file, bool read, bool text, const char* name);
};

}  // namespace distributionally_robust
}  // namespace VW
