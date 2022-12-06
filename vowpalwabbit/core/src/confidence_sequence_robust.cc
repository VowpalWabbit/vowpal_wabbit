// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include "vw/core/model_utils.h"

#include <boost/math/tools/minima.hpp>
#include <cassert>
#include <cmath>

namespace VW
{
g_tilde::g_tilde(double k)
{
  assert(1 < k && k < 2);
  this->k = k;
  this->log_k = std::log(k);
  this->sum_x = 0.0;
  this->sum_low_v = 0.0;
  this->sum_mid_v = 0.0;
  this->t = 0;
}

double g_tilde::histo_variance(std::map<std::pair<uint64_t, bool>, double>& hist, double lam_sqrt_tp1)
{
  double sqrt_tp1 = std::sqrt(t + 1);
  double ret_val = 0.0;
  for (const std::pair<std::pair<uint64_t, bool>, double>& hist_p : hist)
  {
    uint64_t n = hist_p.first.first;
    bool strong_term = hist_p.first.second;
    double c = hist_p.second;
    double x_raw = std::pow(k, n);
    double x_ds = x_raw / sqrt_tp1;
    double curr_val = 0.0;
    if (strong_term) { curr_val = std::pow(lam_sqrt_tp1 * (k - 1) * x_ds / (1 + lam_sqrt_tp1 * k * x_ds), 2); }
    else
    {
      curr_val = lam_sqrt_tp1 * x_ds - std::log1p(lam_sqrt_tp1 * x_ds);
    }
    ret_val += c * curr_val;
  }
  return ret_val;
}

void g_tilde::histo_insert(std::map<std::pair<uint64_t, bool>, double>& hist, double x)
{
  uint64_t n = std::floor(std::log(x) / log_k);
  double x1 = std::pow(k, n);
  double alpha = (k * x1 - x) / ((k - 1.0) * x1);
  // TODO: Kahan summation
  hist[std::make_pair(n, false)] =
      hist.count(std::make_pair(n, false)) ? hist[std::make_pair(n, false)] + alpha : alpha;
  hist[std::make_pair(n + 1, false)] =
      hist.count(std::make_pair(n + 1, false)) ? hist[std::make_pair(n + 1, false)] + 1 - alpha : 1 - alpha;
  hist[std::make_pair(n, true)] = hist.count(std::make_pair(n, true))
      ? hist[std::make_pair(n, true)] + -0.5 * alpha * (1 - alpha)
      : -0.5 * alpha * (1 - alpha);
}

void g_tilde::add_obs(double x)
{
  assert(x >= 0.0);
  double x_hat = (sum_x + 0.5) / (t + 1);
  double error = x - std::min(1.0, x_hat);
  if (error <= 0.0) { sum_low_v += std::pow(error, 2); }
  else if (error <= 1.0)
  {
    sum_mid_v += std::pow(error, 2);
  }
  else
  {
    histo_insert(sum_v_histo, error);
  }

  sum_x += x;
  t += 1;
}

double g_tilde::get_s() { return sum_x; }

double g_tilde::get_v(double lam_sqrt_tp1)
{
  double v_low = sum_low_v / ((t + 1) - std::sqrt(t + 1) * lam_sqrt_tp1);
  double v_mid = sum_mid_v / (t + 1);

  return 0.5 * std::pow(lam_sqrt_tp1, 2) * (v_low + v_mid) + histo_variance(sum_v_histo, lam_sqrt_tp1);
}

countable_discrete_base::countable_discrete_base(double eta, double r, double k, double lambda_max, double xi) : gt(k)
{
  assert(0.0 < eta && eta < 1.0);
  assert(r > 1.0);

  this->eta = eta;
  this->r = r;
#if !defined(__APPLE__) && !defined(_WIN32)
  double zeta_r = std::riemann_zeta(r);
#else
  double zeta_r = 1.6449340668482264;  // std::riemann_zeta(r) -- Assuming r=2.0 is constant
#endif
  this->scale_fac = 0.5 * (1.0 + polylog(r, eta) / (eta * zeta_r));
  assert(0.0 < scale_fac && scale_fac < 1.0);
  this->log_scale_fac = std::log1p(scale_fac - 1.0);
  this->t = 0;
  this->log_xi_m1 = std::log1p(xi - 2.0);

  double lambertw_expression = -0.15859433956303937;  // sc.lambertw(-exp(-2)) in Python
  assert(0.0 < lambda_max && lambda_max <= 1.0 + lambertw_expression);
  assert(1.0 < xi);

  this->lambda_max = lambda_max;
  this->xi = xi;
  this->log_xi = std::log1p(xi - 1);
}

double countable_discrete_base::get_ci(double alpha) { return lb_log_wealth(alpha); }

double countable_discrete_base::get_lam_sqrt_tp1(double j)
{
  double log_den = (j + 0.5) * log_xi - 0.5 * std::log(t + 1);
  return lambda_max * std::exp(-log_den);
}

double countable_discrete_base::get_v_impl(std::map<uint64_t, double>& memo, uint64_t j)
{
  if (memo.find(j) == memo.end()) { memo[j] = get_v(get_lam_sqrt_tp1(j)); }
  return memo[j];
}

double countable_discrete_base::log_sum_exp(std::vector<double> a)
{
  double ret = 0.0;
  for (const double& v : a) { ret += std::exp(v); }
  return std::log(ret);
}

double countable_discrete_base::log_wealth_mix(double mu, double s, double thres, std::map<uint64_t, double>& memo)
{
  double sqrt_tp1 = std::sqrt(t + 1);
  double y = s / sqrt_tp1 - (t / sqrt_tp1) * mu;
  std::vector<double> log_es;
  std::vector<double> log_ws;
  for (const std::pair<uint64_t, double>& memo_p : memo)
  {
    uint64_t j = memo_p.first;
    double v = memo_p.second;
    log_es.push_back(get_lam_sqrt_tp1(j) * y - v);
    log_ws.push_back(get_log_weight(j));
  }

  uint64_t j = log_es.size();
  while (true)
  {
    std::vector<double> combined;
    for (size_t i = 0; i < log_es.size(); ++i) { combined.push_back(log_es[i] + log_ws[i]); }
    double lower_bound = log_sum_exp(combined);

    if (lower_bound >= thres) { return lower_bound; }

    // quasiconcave after the maximum, otherwise lower bound variance with 0
    double max_of_begin = std::numeric_limits<int>::min();
    for (size_t i = 0; i < log_es.size() - 1; ++i) { max_of_begin = std::max(max_of_begin, log_es[i]); }
    double log_upper_exp = (log_es.back() < max_of_begin) ? log_es.back() : get_lam_sqrt_tp1(j) * y;
    double upper_bound = log_sum_exp({lower_bound, get_log_remaining_weight(j) + log_upper_exp});

    if (upper_bound < thres) { return upper_bound; }

    double v = get_v_impl(memo, j);
    log_es.push_back(get_lam_sqrt_tp1(j) * y - v);
    log_ws.push_back(get_log_weight(j));
    j += 1;
  }
}

double countable_discrete_base::root_brentq(
    double s, double thres, std::map<uint64_t, double>& memo, double min_mu, double max_mu)
{
  uint64_t brent_bits = 24;
  auto lw_lambda = [this, &s, &thres, &memo](
                       double mu) -> double { return log_wealth_mix(mu, s, thres, memo) - thres; };
  auto root = boost::math::tools::brent_find_minima(lw_lambda, min_mu, max_mu, brent_bits);
  return root.second;
}

double countable_discrete_base::lb_log_wealth(double alpha)
{
  assert(0.0 < alpha && alpha < 1);
  double thres = -std::log(alpha);

  double min_mu = 0.0;
  double s = get_s();
  std::map<uint64_t, double> memo;
  for (uint64_t j = 0; j < 2; ++j) { memo[j] = get_v(get_lam_sqrt_tp1(j)); }

  double log_wealth_min_mu = log_wealth_mix(min_mu, s, thres, memo);
  if (log_wealth_min_mu <= thres) { return min_mu; }

  double max_mu = 1.0;
  double log_wealth_max_mu = log_wealth_mix(max_mu, s, thres, memo);
  if (log_wealth_max_mu <= thres) { return max_mu; }
  return root_brentq(s, thres, memo, min_mu, max_mu);
}

double countable_discrete_base::polylog(double r, double eta)
{
  double ret_val = 0.0;
  double min_thres = 1e-10;
  double curr_val = std::numeric_limits<int>::max();
  uint64_t k = 0;
  while (curr_val > min_thres)
  {
    k += 1;
    curr_val = std::pow(eta, k) / std::pow(k, r);
    ret_val += curr_val;
  }
  return ret_val;
}

double countable_discrete_base::get_log_weight(double j) { return log_scale_fac + log_xi_m1 - (1 + j) * log_xi; }

double countable_discrete_base::get_log_remaining_weight(double j) { return log_scale_fac - j * log_xi; }

double countable_discrete_base::get_s() { return gt.get_s(); }

double countable_discrete_base::get_v(double lam_sqrt_tp1) { return gt.get_v(lam_sqrt_tp1); }

void countable_discrete_base::add_obs(double x)
{
  t += 1;
  gt.add_obs(x);
}

void off_policy_cs::add_obs(double w, double r)
{
  assert(w >= 0.0);
  assert(0.0 <= r && r <= 1.0);
  lower.add_obs(w * r);
  upper.add_obs(w * (1.0 - r));
}

std::pair<double, double> off_policy_cs::get_ci(double alpha)
{
  return std::make_pair(lower.get_ci(alpha / 2.0), upper.get_ci(alpha / 2.0));
}

namespace model_utils
{
}  // namespace model_utils
}  // namespace VW