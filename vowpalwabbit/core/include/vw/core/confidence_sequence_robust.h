// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#if !defined(__APPLE__) && !defined(_WIN32)
#  define __STDCPP_MATH_SPEC_FUNCS__ 201003L
#  define __STDCPP_WANT_MATH_SPEC_FUNCS__ 1
#endif

#include "vw/core/io_buf.h"
#include "vw/core/metric_sink.h"

namespace VW
{
class lower_cs_base
{
public:
  void add_obs(double x);
  double get_ci(double alpha);
};

class countable_discrete_base : public lower_cs_base
{
public:
  countable_discrete_base(double lambda_max = 0.5, double xi = 1.6);
  double lambertw(double x);
  double get_ci(double alpha);
  double get_lam_sqrt_tp1(double j);
  double get_log_weight(double j);
  double get_log_remaining_weight(double j);
  double get_v_impl(std::map<uint64_t, double>& memo, uint64_t j);
  double log_sum_exp(std::vector<double> a);
  double log_wealth_mix(double mu, double s, double thres, std::map<uint64_t, double>& memo);
  double root_brentq(double s, double thres, std::map<uint64_t, double>& memo, double min_mu, double max_mu);
  double lb_log_wealth(double alpha);
  double get_s();
  double get_v(double lam_sqrt_tp1);

  double xi;
  uint64_t t;
  double log_xi;

private:
  double lambda_max;
};

class g_tilde
{
public:
  g_tilde(double k);
  double histo_variance(std::map<std::pair<uint64_t, bool>, double>& hist, double lam_sqrt_tp1);
  void histo_insert(std::map<std::pair<uint64_t, bool>, double>& hist, double x);
  void add_obs(double x);
  double get_s();
  double get_v(double lam_sqrt_tp1);

private:
  double k;
  double log_k;
  double sum_x;
  double sum_low_v;
  double sum_mid_v;
  std::map<std::pair<uint64_t, bool>, double> sum_v_histo;
  uint64_t t;
};

class robust_mixture : public countable_discrete_base
{
public:
  robust_mixture(double eta = 0.95f, double r = 2, double k = 1.5);
  double get_log_weight(double j);
  double get_log_remaining_weight(double j);
  double get_s();
  double get_v(double lam_sqrt_tp1);
  double add_obs(double x);

private:
  double eta;
  double r;
  g_tilde gt;
  double scale_fac;
  double log_scale_fac;
  double log_xi_m1;
};

class off_policy_cs
{
public:
  off_policy_cs() = default;
  void add_obs(double w, double r);
  std::pair<double, double> get_ci(double alpha);

private:
  robust_mixture lower;
  robust_mixture upper;
};

namespace model_utils
{
/*size_t read_model_field(io_buf&, VW::incremental_f_sum&);
size_t write_model_field(io_buf&, const VW::incremental_f_sum&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::confidence_sequence&);
size_t write_model_field(io_buf&, const VW::confidence_sequence&, const std::string&, bool);*/
}  // namespace model_utils
}  // namespace VW
