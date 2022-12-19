// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence_robust.h"

#include "vw/core/model_utils.h"

#include <cassert>
#include <cmath>

namespace VW
{
g_tilde::g_tilde(double k) : k(k), log_k(std::log(k)), sum_x(0.0), sum_low_v(0.0), sum_mid_v(0.0), t(0)
{
  assert(1 < k && k < 2);
}

double g_tilde::histo_variance(double lam_sqrt_tp1) const
{
  double sqrt_tp1 = std::sqrt(t + 1);
  double ret_val = 0.0;
  for (const std::pair<std::pair<uint64_t, bool>, double>& hist_p : sum_v_histo)
  {
    uint64_t n = hist_p.first.first;
    bool strong_term = hist_p.first.second;
    double c = hist_p.second;
    double x_raw = std::pow(k, n);
    double x_ds = x_raw / sqrt_tp1;
    double curr_val = 0.0;
    if (strong_term) { curr_val = std::pow(lam_sqrt_tp1 * (k - 1) * x_ds / (1 + lam_sqrt_tp1 * k * x_ds), 2); }
    else { curr_val = lam_sqrt_tp1 * x_ds - std::log1p(lam_sqrt_tp1 * x_ds); }
    ret_val += c * curr_val;
  }
  return ret_val;
}

void g_tilde::histo_insert(double x)
{
  uint64_t n = std::floor(std::log(x) / log_k);
  double x1 = std::pow(k, n);
  double alpha = (k * x1 - x) / ((k - 1.0) * x1);
  // TODO: Kahan summation
  sum_v_histo[std::make_pair(n, false)] =
      sum_v_histo.count(std::make_pair(n, false)) ? sum_v_histo[std::make_pair(n, false)] + alpha : alpha;
  sum_v_histo[std::make_pair(n + 1, false)] = sum_v_histo.count(std::make_pair(n + 1, false))
      ? sum_v_histo[std::make_pair(n + 1, false)] + 1 - alpha
      : 1 - alpha;
  sum_v_histo[std::make_pair(n, true)] = sum_v_histo.count(std::make_pair(n, true))
      ? sum_v_histo[std::make_pair(n, true)] + -0.5 * alpha * (1 - alpha)
      : -0.5 * alpha * (1 - alpha);
}

void g_tilde::add_obs(double x)
{
  assert(x >= 0.0);
  double x_hat = (sum_x + 0.5) / (t + 1);
  double error = x - std::min(1.0, x_hat);
  if (error <= 0.0) { sum_low_v += std::pow(error, 2); }
  else if (error <= 1.0) { sum_mid_v += std::pow(error, 2); }
  else { histo_insert(error); }

  sum_x += x;
  t += 1;
}

double g_tilde::get_s() const { return sum_x; }

double g_tilde::get_v(double lam_sqrt_tp1) const
{
  double v_low = sum_low_v / ((t + 1) - std::sqrt(t + 1) * lam_sqrt_tp1);
  double v_mid = sum_mid_v / (t + 1);

  return 0.5 * std::pow(lam_sqrt_tp1, 2) * (v_low + v_mid) + histo_variance(lam_sqrt_tp1);
}

void g_tilde::reset_stats()
{
  sum_x = 0.0;
  sum_low_v = 0.0;
  sum_mid_v = 0.0;
  sum_v_histo.clear();
  t = 0;
}

countable_discrete_base::countable_discrete_base(double eta, double r, double k, double lambda_max, double xi)
    : eta(eta)
    , r(r)
    , xi(xi)
    , log_xi(std::log1p(xi - 1))
    , log_xi_m1(std::log1p(xi - 2.0))
    , lambda_max(lambda_max)
    , zeta_r(
#if !defined(__APPLE__) && !defined(_WIN32)
          std::riemann_zeta(r)
#else
          1.6449340668482264  // std::riemann_zeta(r) -- Assuming r=2.0 is constant
#endif
              )
    , scale_fac(0.5 * (1.0 + polylog(r, eta) / (eta * zeta_r)))
    , log_scale_fac(std::log1p(scale_fac - 1.0))
    , t(0)
    , gt(k)
{
  assert(0.0 < eta && eta < 1.0);
  assert(r > 1.0);
  assert(0.0 < scale_fac && scale_fac < 1.0);
  double lambertw_expression = -0.15859433956303937;  // sc.lambertw(-exp(-2)) in Python
  assert(0.0 < lambda_max && lambda_max <= 1.0 + lambertw_expression);
  assert(1.0 < xi);
}

double countable_discrete_base::get_ci(double alpha) const { return lb_log_wealth(alpha); }

double countable_discrete_base::get_lam_sqrt_tp1(double j) const
{
  double log_den = (j + 0.5) * log_xi - 0.5 * std::log(t + 1);
  return lambda_max * std::exp(-log_den);
}

double countable_discrete_base::get_v_impl(std::map<uint64_t, double>& memo, uint64_t j) const
{
  if (memo.find(j) == memo.end()) { memo[j] = get_v(get_lam_sqrt_tp1(j)); }
  return memo[j];
}

double countable_discrete_base::log_sum_exp(const std::vector<double>& a) const
{
  double max_exp = *std::max_element(a.begin(), a.end());
  double ret = 0.0;
  for (const double& v : a) { ret += std::exp(v - max_exp); }

  return std::log(ret) + max_exp;
}

double countable_discrete_base::log_wealth_mix(
    double mu, double s, double thres, std::map<uint64_t, double>& memo) const
{
  double sqrt_tp1 = std::sqrt(t + 1);
  double y = s / sqrt_tp1 - (t / sqrt_tp1) * mu;
  std::vector<double> log_es;
  std::vector<double> log_ws;
  for (const std::pair<const unsigned long, double>& memo_p : memo)
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
    double upper_bound = log_sum_exp(std::vector<double>({lower_bound, get_log_remaining_weight(j) + log_upper_exp}));

    if (upper_bound < thres) { return upper_bound; }

    double v = get_v_impl(memo, j);
    log_es.push_back(get_lam_sqrt_tp1(j) * y - v);
    log_ws.push_back(get_log_weight(j));
    j += 1;
  }
}

double countable_discrete_base::root_brentq(
    double s_0, double thres, std::map<uint64_t, double>& memo, double a, double b, double toll_x, double toll_f) const
{
  auto f = [this, &s_0, &thres, &memo](double mu) -> double { return log_wealth_mix(mu, s_0, thres, memo) - thres; };
  double fa = f(a);
  double fb = f(b);
  double fs;

  if (!(fa * fb < 0))
  {
    // std::cerr << "Signs of f(x_min) and f(x_max) must be opposites" << std::endl;
    // return 0;
  }

  if (std::abs(fa) < std::abs(b))
  {
    std::swap(a, b);
    std::swap(fa, fb);
  }

  double c = a;
  double fc = fa;
  bool mflag = true;
  double s = 0;
  double d = 0;

  unsigned int iter = 0;
  while (std::abs(fc) > toll_f && std::abs(b - a) > toll_x)
  {
    ++iter;

    if (fa != fc && fb != fc)  // use inverse quadratic interopolation
    {
      s = (a * fb * fc / ((fa - fb) * (fa - fc))) + (b * fa * fc / ((fb - fa) * (fb - fc))) +
          (c * fa * fb / ((fc - fa) * (fc - fb)));
    }
    else  // secant method
    {
      s = b - fb * (b - a) / (fb - fa);
    }

    if (((s < (3 * a + b) / 4.) || (s > b)) || (mflag && (std::abs(s - b) >= (std::abs(b - c) * 0.5))) ||
        (!mflag && (std::abs(s - b) >= (std::abs(c - d) * 0.5))) || (mflag && (std::abs(b - c) < toll_x)) ||
        (!mflag && (std::abs(c - d) < toll_x)))
    {
      // bisection method
      s = .5 * (a + b);
      mflag = true;
    }
    else { mflag = false; }

    fs = f(s);
    d = c;
    c = b;
    fc = fb;

    if (fa * fs < 0)
    {
      b = s;
      fb = fs;
    }
    else
    {
      a = s;
      fa = fs;
    }

    if (std::abs(fa) < std::abs(fb))
    {
      std::swap(a, b);
      std::swap(fa, fb);
    }
  }

  return s;
}

double countable_discrete_base::lb_log_wealth(double alpha) const
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
  if (log_wealth_max_mu >= thres) { return max_mu; }
  return root_brentq(s, thres, memo, min_mu, max_mu);
}

double countable_discrete_base::polylog(double r, double eta) const
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

double countable_discrete_base::get_log_weight(double j) const { return log_scale_fac + log_xi_m1 - (1 + j) * log_xi; }

double countable_discrete_base::get_log_remaining_weight(double j) const { return log_scale_fac - j * log_xi; }

double countable_discrete_base::get_s() const { return gt.get_s(); }

double countable_discrete_base::get_v(double lam_sqrt_tp1) const { return gt.get_v(lam_sqrt_tp1); }

void countable_discrete_base::add_obs(double x)
{
  t += 1;
  gt.add_obs(x);
}

void countable_discrete_base::reset_stats()
{
  t = 0;
  gt.reset_stats();
}

confidence_sequence_robust::confidence_sequence_robust(double alpha)
    : alpha(alpha), update_count(0), last_w(0.0), last_r(0.0)
{}

void confidence_sequence_robust::update(double w, double r)
{
  assert(w >= 0.0);
  assert(0.0 <= r && r <= 1.0);
  lower.add_obs(w * r);
  upper.add_obs(w * (1.0 - r));
  ++update_count;
  last_w = w;
  last_r = r;
}

void confidence_sequence_robust::persist(metric_sink& metrics, const std::string& suffix)
{
  metrics.set_uint("upcnt" + suffix, update_count);
  metrics.set_float("lb" + suffix, lower_bound());
  metrics.set_float("ub" + suffix, upper_bound());
  metrics.set_float("last_w" + suffix, last_w);
  metrics.set_float("last_r" + suffix, last_r);
}

void confidence_sequence_robust::reset_stats()
{
  lower.reset_stats();
  upper.reset_stats();
  update_count = 0;
  last_w = 0.0;
  last_r = 0.0;
}

double confidence_sequence_robust::lower_bound() const { return lower.get_ci(alpha / 2.0); }

double confidence_sequence_robust::upper_bound() const { return 1 - upper.get_ci(alpha / 2.0); }

namespace model_utils
{
}  // namespace model_utils
}  // namespace VW