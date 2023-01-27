// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/vw_fwd.h"

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace VW
{
namespace details
{
constexpr double CS_ROBUST_DEFAULT_ALPHA = 0.05f;

// Equivalent to GTilde in confidence_sequence_robust.py
class g_tilde
{
public:
  g_tilde(double k);
  double histo_variance(double lam_sqrt_tp1) const;
  void histo_insert(double x);
  void add_obs(double x);
  double get_s() const;
  double get_v(double lam_sqrt_tp1) const;
  void reset_stats();

  // Constant values -- do not require reset or save_load
  double k;
  double log_k;

  double sum_x;
  double sum_low_v;
  double sum_mid_v;
  uint64_t t;
  std::map<std::pair<uint64_t, bool>, double> sum_v_histo;
};

// Equivalent to CountableDiscreteBase in confidence_sequence_robust.py
class countable_discrete_base
{
public:
  countable_discrete_base(double tol_x = 1e-6, bool is_brentq = false, double eta = 0.95f, double k = 1.5,
      double lambda_max = 0.5, double xi = 1.6);
  double get_ci(double alpha) const;
  double get_lam_sqrt_tp1(double j) const;
  double get_v_impl(std::map<uint64_t, double>& memo, uint64_t j) const;
  double log_wealth_mix(double mu, double s, double thres, std::map<uint64_t, double>& memo) const;
  double root_bisect(double s, double thres, std::map<uint64_t, double>& memo, double min_mu, double max_mu) const;
  double root_brentq(double s, double thres, std::map<uint64_t, double>& memo, double min_mu, double max_mu) const;
  double log_sum_exp(const std::vector<double>& combined) const;
  double lb_log_wealth(double alpha) const;
  double get_log_weight(double j) const;
  double get_log_remaining_weight(double j) const;
  double get_s() const;
  double get_v(double lam_sqrt_tp1) const;
  void add_obs(double x);
  void reset_stats();

  // Constant values -- do not require reset or save_load
  double tol_x;
  bool is_brentq;
  double log_xi;
  double log_xi_m1;
  double lambda_max;
  double zeta_r;
  double scale_fac;
  double log_scale_fac;

  uint64_t t;
  g_tilde gt;
};
}  // namespace details

namespace estimators
{
// Equivalent to DDRM in confidence_sequence_robust.py
class confidence_sequence_robust
{
public:
  confidence_sequence_robust(
      double tol_x = 1e-6, bool is_brentq = false, double alpha = VW::details::CS_ROBUST_DEFAULT_ALPHA);
  void update(double w, double r);
  void persist(metric_sink& metrics, const std::string& suffix);
  void reset_stats();
  double lower_bound() const;
  double upper_bound() const;

  // Constant values -- do not require reset or save_load
  double alpha;

  uint64_t update_count;
  double last_w;
  double last_r;
  VW::details::countable_discrete_base lower;
  VW::details::countable_discrete_base upper;
};
}  // namespace estimators

namespace model_utils
{
size_t read_model_field(io_buf&, VW::details::g_tilde&);
size_t write_model_field(io_buf&, const VW::details::g_tilde&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::details::countable_discrete_base&);
size_t write_model_field(io_buf&, const VW::details::countable_discrete_base&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::estimators::confidence_sequence_robust&);
size_t write_model_field(io_buf&, const VW::estimators::confidence_sequence_robust&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
