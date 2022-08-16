// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <algorithm>
#include "vw/core/io_buf.h"

constexpr float DEFAULT_ALPHA = 0.05f;

namespace VW
{
namespace confidence_sequence
{
struct IntervalImpl
{
    double rho = 1.0;
    double rmin = 0.0;
    double rmax = 1.0;
    bool adjust = true;

    int t = 0;

    double sumwsqrsq = incrementalfsum();
    double sumwsqr = incrementalfsum();
    double sumwsq = incrementalfsum();
    double sumwr = incrementalfsum();
    double sumw = incrementalfsum();
    double sumwrxhatlow = incrementalfsum();
    double sumwxhatlow = incrementalfsum();
    double sumxhatlowsq = incrementalfsum();
    double sumwrxhathigh = incrementalfsum();
    double sumwxhathigh = incrementalfsum();
    double sumxhathighsq = incrementalfsum();

    double incrementalfsum();
    double loggamma(double a);
    double gammainc(double a, double x);
    double loggammalowerinc(double a, double x);
    double logwealth(double s, double v, double rho);
    std::pair<bool, double> root_scalar(double sumXt, double v, double rho, double thres, double minmu, double maxmu);
    double lblogwealth(double sumXt, double v, double rho, double alpha);
    IntervalImpl(double rmin, double rmax, bool adjust);
    void add(double w, double r);
    std::pair<double, double> get(double alpha = DEFAULT_ALPHA);

  // alpha: confidence level
  // tau: count decay time constant
  /*explicit ChiSquared(double _alpha, double _tau, double _wmin = 0.0,
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
  void save_load(io_buf& model_file, bool read, bool text, const char* name);*/
};
}  // namespace confidence_sequence

namespace model_utils
{
size_t read_model_field(io_buf&, VW::confidence_sequence::IntervalImpl&);
size_t write_model_field(io_buf&, const VW::confidence_sequence::IntervalImpl&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
