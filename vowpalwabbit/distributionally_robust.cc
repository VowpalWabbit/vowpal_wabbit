#include <cmath>
#include <list>
#include <type_traits>

#include "distributionally_robust.h"
#include "vw_math.h"
// #include "model_utils.h"

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, VW::distributionally_robust::Duals& duals)
{
  if (io.num_files() == 0) { return 0; }
  size_t bytes = 0;
  bytes += read_model_field(io, duals.unbounded);
  bytes += read_model_field(io, duals.kappa);
  bytes += read_model_field(io, duals.gamma);
  bytes += read_model_field(io, duals.beta);
  bytes += read_model_field(io, duals.n);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::distributionally_robust::Duals& duals, const std::string&, bool text)
{
  if (io.num_files() == 0) { return 0; }
  size_t bytes = 0;
  bytes += write_model_field(io, duals.unbounded, "_duals_unbounded", text);
  bytes += write_model_field(io, duals.kappa, "_duals_kappa", text);
  bytes += write_model_field(io, duals.gamma, "_duals_gamma", text);
  bytes += write_model_field(io, duals.beta, "_duals_beta", text);
  bytes += write_model_field(io, duals.n, "_duals_n", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::distributionally_robust::ChiSquared& chisq)
{
  if (io.num_files() == 0) { return 0; }
  size_t bytes = 0;
  bytes += read_model_field(io, chisq.duals.first);
  bytes += read_model_field(io, chisq.duals_stale);
  bytes += read_model_field(io, chisq.duals.second);
  bytes += read_model_field(io, chisq.alpha);
  bytes += read_model_field(io, chisq.tau);
  bytes += read_model_field(io, chisq.wmin);
  bytes += read_model_field(io, chisq.wmax);
  bytes += read_model_field(io, chisq.rmin);
  bytes += read_model_field(io, chisq.rmax);
  bytes += read_model_field(io, chisq.n);
  bytes += read_model_field(io, chisq.sumw);
  bytes += read_model_field(io, chisq.sumwsq);
  bytes += read_model_field(io, chisq.sumwr);
  bytes += read_model_field(io, chisq.sumwsqr);
  bytes += read_model_field(io, chisq.sumwsqrsq);
  bytes += read_model_field(io, chisq.delta);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::distributionally_robust::ChiSquared& chisq, const std::string&, bool text)
{
  if (io.num_files() == 0) { return 0; }
  size_t bytes = 0;
  bytes += write_model_field(io, chisq.duals.first, "_chisq_chi_scored", text);
  bytes += write_model_field(io, chisq.duals_stale, "_chisq_chi_duals_stale", text);
  bytes += write_model_field(io, chisq.duals.second, "", text);
  bytes += write_model_field(io, chisq.alpha, "_chisq_chi_alpha", text);
  bytes += write_model_field(io, chisq.tau, "_chisq_chi_tau", text);
  bytes += write_model_field(io, chisq.wmin, "_chisq_chi_wmin", text);
  bytes += write_model_field(io, chisq.wmax, "_chisq_chi_wmax", text);
  bytes += write_model_field(io, chisq.rmin, "_chisq_chi_rmin", text);
  bytes += write_model_field(io, chisq.rmax, "_chisq_chi_rmax", text);
  bytes += write_model_field(io, chisq.n, "_chisq_chi_n", text);
  bytes += write_model_field(io, chisq.sumw, "_chisq_chi_sumw", text);
  bytes += write_model_field(io, chisq.sumwsq, "_chisq_chi_sumwsq", text);
  bytes += write_model_field(io, chisq.sumwr, "_chisq_chi_sumwr", text);
  bytes += write_model_field(io, chisq.sumwsqr, "_chisq_chi_sumwsqr", text);
  bytes += write_model_field(io, chisq.sumwsqrsq, "_chisq_chi_sumwsqrsq", text);
  bytes += write_model_field(io, chisq.delta, "_chisq_chi_delta", text);
  return bytes;
}
}  // namespace model_utils

namespace distributionally_robust
{
double ChiSquared::chisq_onedof_isf(double alpha)
{
  // the following is a polynomial approximation to the
  // inverse survival function for chi-squared distribution with 1 dof
  // using log and exp as basis functions
  // "constants" below found with the following Mathematica code
  //
  // data = Table[{ alpha, InverseCDF[ChiSquareDistribution[1], 1 - alpha] }, { alpha, 0.001, 0.999, 0.0005 }]
  // lm = LinearModelFit[data, { Log[alpha], Log[alpha]^2, Log[alpha]^3, Log[alpha]^4 , Log[alpha]^5, Log[alpha]^6,
  // Log[alpha]^7, Log[alpha]^8, Exp[alpha], Exp[alpha]^2, Exp[alpha]^3, Exp[alpha]^4, Exp[alpha]^5, Exp[alpha]^6,
  // Exp[alpha]^7, Exp[alpha]^8}, alpha] ListPlot[lm["FitResiduals"]] lm["BestFitParameters"]
  // Show[Plot[InverseCDF[ChiSquareDistribution[1], 1 - alpha], { alpha, 0 , 1}],  Plot[lm[alpha], { alpha, 0, 1 }],
  // Frame->True]

  constexpr double constants[] = {-1.73754, -1.40684, 0.0758363, 0.00726577, 0.000468688, 0.0000214395, 1.0643e-6,
      6.43011e-8, 2.0475e-9, 1.16356, -0.575446, 0.329796, -0.136076, 0.0396764, -0.00763232, 0.00087113,
      -0.0000445128};

  double logalpha = std::log(alpha);
  double expalpha = std::exp(alpha);
  double logalphapow = logalpha;
  double expalphapow = expalpha;

  double rv = constants[0];
  for (int i = 1; i <= 8; ++i)
  {
    rv += constants[i] * logalphapow;
    rv += constants[i + 8] * expalphapow;
    logalphapow *= logalpha;
    expalphapow *= expalpha;
  }

  return rv;
}

static bool isclose(double x, double y, double atol = 1e-8)
{
  double rtol = 1e-5;

  return std::abs(x - y) <= (atol + rtol * std::abs(y));
}

ScoredDual ChiSquared::recompute_duals()
{
  if (n <= 0)
  {
    duals = std::make_pair(rmin, Duals(true, 0, 0, 0, 0));

    return duals;
  }

  double uncwfake = sumw < n ? wmax : wmin;
  double uncgstar;

  if (uncwfake == std::numeric_limits<double>::infinity()) { uncgstar = 1.0 + 1.0 / n; }
  else
  {
    double unca = (uncwfake + sumw) / (1 + n);
    double uncb = (uncwfake * uncwfake + sumwsq) / (1 + n);

    // NB: (uncb > unca * unca) is guaranteed
    uncgstar = (n + 1) * (unca - 1) * (unca - 1) / (uncb - unca * unca);
  }

  double phi = (-uncgstar - delta) / (2 * (n + 1));

  double r = rmin;
  double sign = 1;

  std::list<ScoredDual> candidates;

  for (auto wfake : {wmin, wmax})
  {
    if (wfake == std::numeric_limits<double>::infinity())
    {
      double x = r + (sumwr - sumw * r) / n;
      double y = (r * sumw - sumwr) * (r * sumw - sumwr) / (n * (1 + n)) -
          (r * r * sumwsq - 2 * r * sumwsqr + sumwsqrsq) / (1 + n);
      double z = phi + 1 / (2 * n);

      if (isclose(y * z, 0, 1e-9)) { y = 0; }

      if (z <= 0 && y * z >= 0)
      {
        double kappa = std::sqrt(y / (2 * z));

        if (isclose(kappa, 0))
        {
          Duals candidate = {true, 0, 0, 0, n};
          candidates.push_back(std::make_pair(r, candidate));
        }
        else
        {
          double gstar = x - std::sqrt(2 * y * z);
          double gamma = -kappa * (1 + n) / n + (r * sumw - sumwr) / n;
          double beta = -sign * r;
          Duals candidate = {false, kappa, gamma, beta, n};
          candidates.push_back(std::make_pair(gstar, candidate));
        }
      }
    }
    else
    {
      double barw = (wfake + sumw) / (1 + n);
      double barwsq = (wfake * wfake + sumwsq) / (1 + n);
      double barwr = sign * (wfake * r + sumwr) / (1 + n);
      double barwsqr = sign * (wfake * wfake * r + sumwsqr) / (1 + n);
      double barwsqrsq = (wfake * wfake * r * r + sumwsqrsq) / (1 + n);

      if (barwsq > barw * barw)
      {
        double x = barwr + ((1 - barw) * (barwsqr - barw * barwr) / (barwsq - barw * barw));
        double y =
            (barwsqr - barw * barwr) * (barwsqr - barw * barwr) / (barwsq - barw * barw) - (barwsqrsq - barwr * barwr);
        double z = phi + 0.5 * (1 - barw) * (1 - barw) / (barwsq - barw * barw);

        if (isclose(y * z, 0, 1e-9)) { y = 0; }

        if (z <= 0 && y * z >= 0)
        {
          double kappa = std::sqrt(y / (2 * z));

          if (isclose(kappa, 0))
          {
            Duals candidate = {true, 0, 0, 0, n};
            candidates.push_back(std::make_pair(r, candidate));
          }
          else
          {
            double gstar = x - std::sqrt(2 * y * z);
            double beta = (-kappa * (1 - barw) - (barwsqr - barw * barwr)) / (barwsq - barw * barw);
            double gamma = -kappa - beta * barw - barwr;
            Duals candidate = {false, kappa, gamma, beta, n};
            candidates.push_back(std::make_pair(gstar, candidate));
          }
        }
      }
    }
  }

  if (candidates.empty()) { duals = std::make_pair(rmin, Duals(true, 0, 0, 0, n)); }
  else
  {
    auto it = std::min_element(candidates.begin(), candidates.end(),
        [](const ScoredDual& x, const ScoredDual& y) { return std::get<0>(x) < std::get<0>(y); });

    duals = *it;
  }

  duals.first = VW::math::clamp(sign * duals.first, rmin, rmax);

  return duals;
}

void ChiSquared::save_load(io_buf& model_file, bool read, bool text, const char* name)
{
  if (model_file.num_files() == 0) { return; }

#define save_load_field(field)                                                                            \
  do                                                                                                      \
  {                                                                                                       \
    if (read) { VW::model_utils::read_model_field(model_file, field); }                                   \
    else                                                                                                  \
    {                                                                                                     \
      VW::model_utils::write_model_field(model_file, field, fmt::format("{}_chisq_" #field, name), text); \
    }                                                                                                     \
  } while (0)

  save_load_field(n);
  save_load_field(sumw);
  save_load_field(sumwsq);
  save_load_field(sumwr);
  save_load_field(sumwsqr);
  save_load_field(sumwsqrsq);

  save_load_field(rmin);
  save_load_field(rmax);
  save_load_field(wmin);
  save_load_field(wmax);

  duals_stale = true;
}

}  // namespace distributionally_robust

}  // namespace VW
