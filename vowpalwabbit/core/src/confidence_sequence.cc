// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/confidence_sequence.h"

#include "vw/core/model_utils.h"

#include <cassert>
#include <cmath>

namespace VW
{
namespace confidence_sequence
{
double IntervalImpl::approxpolygammaone(double b)
{
  assert(b >= 1.0);
  if (b > 10.0)
  {
    double z = 1.0 / b;
    // Assuming[z > 0, FullSimplify@Series[PolyGamma[1, 1/z], { z, 0, 8 }]]
    return z * (1.0 + z * (0.5 + z * (1.0 / 6.0 + std::pow(z, 2) * (-1.0 / 30.0 + std::pow(z, 2) * (1.0 / 42.0)))));
  }
  else
  {
    // MiniMaxApproximation[PolyGamma[1, z], { z, { 1, 10 }, 4, 4 }]
    double numerator = 52.654073150313565 +
        b * (176.21984028201697 + b * (230.4407865623894 + b * (232.30138091080408 + b * (0.00006342628796017858))));
    double denominator =
        1.0 + b * (-7.709336904239235 + b * (80.59885744577618 + b * (114.25915616534267 + b * (232.30352278139097))));
    return numerator / denominator;
  }
}

double IntervalImpl::lb_new(double sumXt, double v, double eta, double s, double alpha)
{
  double zeta_s = std::riemann_zeta(s);
  v = std::max(v, 1.0);
  double gamma1 = (std::pow(eta, 0.25) + std::pow(eta, 0.25)) / std::sqrt(2.0);
  double gamma2 = (std::sqrt(eta) + 1.0) / 2.0;
  assert(((std::log(eta * v) / std::log(eta)) + 1.0 > 0.0) && (1.0 + (std::log(eta * v) / std::log(eta)) != 0.0));
  double ll = s * std::log((std::log(eta * v) / std::log(eta)) + 1.0) + std::log(zeta_s / alpha);
  return std::max(
      0.0, (sumXt - std::sqrt(std::pow(gamma1, 2) * ll * v + std::pow(gamma2, 2) * std::pow(ll, 2)) - gamma2 * ll) / t);
}

IntervalImpl::IntervalImpl(double rmin, double rmax, bool adjust) : rmin(rmin), rmax(rmax), adjust(adjust) {}

void IntervalImpl::addobs(double w, double r, double p_drop, double n_drop)
{
  assert(w >= 0.0);
  assert(0.0 <= p_drop && p_drop < 1.0);
  assert(n_drop == -1.0 || n_drop >= 0.0);  // -1.0 equivalent to None in Python code

  if (!adjust) { r = std::min(rmax, std::max(rmin, r)); }
  else
  {
    rmin = std::min(rmin, r);
    rmax = std::max(rmax, r);
  }

  if (n_drop == -1.0) { n_drop = p_drop / (1.0 - p_drop); }

  double sumXlow = 0.0;
  double Xhatlow = 0.0;
  double sumXhigh = 0.0;
  double Xhathigh = 0.0;

  if (n_drop > 0.0)
  {
    sumXlow = (sumwr - sumw * rmin) / (rmax - rmin);
    double alow = sumXlow + 0.5;
    double blow = t + 1.0;
    sumxhatlowsq += std::pow(alow, 2) * (approxpolygammaone(blow) - approxpolygammaone(blow + n_drop));

    sumXhigh = (sumw * rmax - sumwr) / (rmax - rmin);
    double ahigh = sumXhigh + 0.5;
    double bhigh = t + 1.0;
    sumxhathighsq += std::pow(ahigh, 2) * (approxpolygammaone(bhigh) - approxpolygammaone(bhigh + n_drop));

    t += n_drop;
  }

  sumXlow = (sumwr - sumw * rmin) / (rmax - rmin);
  Xhatlow = (sumXlow + 0.5) / (t + 1.0);
  sumXhigh = (sumw * rmax - sumwr) / (rmax - rmin);
  Xhathigh = (sumXhigh + 0.5) / (t + 1.0);

  w /= (1.0 - p_drop);

  sumwsqrsq += std::pow(w * r, 2);
  sumwsqr += std::pow(w, 2) * r;
  sumwsq += std::pow(w, 2);
  sumwr += w * r;
  sumw += w;
  sumwrxhatlow += w * r * Xhatlow;
  sumwxhatlow += w * Xhatlow;
  sumxhatlowsq += std::pow(Xhatlow, 2);
  sumwrxhathigh += w * r * Xhathigh;
  sumwxhathigh += w * Xhathigh;
  sumxhathighsq += std::pow(Xhathigh, 2);

  ++t;
}

std::pair<double, double> IntervalImpl::getci(double alpha)
{
  if (t == 0 || rmin == rmax) { return std::make_pair(rmin, rmax); }

  double sumvlow = (sumwsqrsq - 2.0 * rmin * sumwsqr + std::pow(rmin, 2) * sumwsq) / std::pow(rmax - rmin, 2) -
      2.0 * (sumwrxhatlow - rmin * sumwxhatlow) / (rmax - rmin) + sumxhatlowsq;
  double sumXlow = (sumwr - sumw * rmin) / (rmax - rmin);
  double l = lb_new(sumXlow, sumvlow, eta, s, alpha / 2.0);

  double sumvhigh = (sumwsqrsq - 2.0 * rmax * sumwsqr + std::pow(rmax, 2) * sumwsq) / std::pow(rmax - rmin, 2) +
      2.0 * (sumwrxhathigh - rmax * sumwxhathigh) / (rmax - rmin) + sumxhathighsq;
  double sumXhigh = (sumw * rmax - sumwr) / (rmax - rmin);
  double u = 1.0 - lb_new(sumXhigh, sumvhigh, eta, s, alpha / 2.0);

  return std::make_pair(rmin + l * (rmax - rmin), rmin + u * (rmax - rmin));
}
}  // namespace confidence_sequence

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::confidence_sequence::IncrementalFsum& ifs)
{
  size_t bytes = 0;
  bytes += read_model_field(io, ifs.partials);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::confidence_sequence::IncrementalFsum& ifs, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, ifs.partials, upstream_name + "_partials", text);
  return bytes;
}

size_t read_model_field(io_buf& io, VW::confidence_sequence::IntervalImpl& im)
{
  size_t bytes = 0;
  bytes += read_model_field(io, im.eta);
  bytes += read_model_field(io, im.s);
  bytes += read_model_field(io, im.rmin);
  bytes += read_model_field(io, im.rmax);
  bytes += read_model_field(io, im.adjust);
  bytes += read_model_field(io, im.t);
  bytes += read_model_field(io, im.sumwsqrsq);
  bytes += read_model_field(io, im.sumwsqr);
  bytes += read_model_field(io, im.sumwsq);
  bytes += read_model_field(io, im.sumwr);
  bytes += read_model_field(io, im.sumw);
  bytes += read_model_field(io, im.sumwrxhatlow);
  bytes += read_model_field(io, im.sumwxhatlow);
  bytes += read_model_field(io, im.sumxhatlowsq);
  bytes += read_model_field(io, im.sumwrxhathigh);
  bytes += read_model_field(io, im.sumwxhathigh);
  bytes += read_model_field(io, im.sumxhathighsq);
  return bytes;
}

size_t write_model_field(
    io_buf& io, const VW::confidence_sequence::IntervalImpl& im, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, im.eta, upstream_name + "_eta", text);
  bytes += write_model_field(io, im.s, upstream_name + "_s", text);
  bytes += write_model_field(io, im.rmin, upstream_name + "_rmin", text);
  bytes += write_model_field(io, im.rmax, upstream_name + "_rmax", text);
  bytes += write_model_field(io, im.adjust, upstream_name + "_adjust", text);
  bytes += write_model_field(io, im.t, upstream_name + "_t", text);
  bytes += write_model_field(io, im.sumwsqrsq, upstream_name + "_sumwsqrsq", text);
  bytes += write_model_field(io, im.sumwsqr, upstream_name + "_sumwsqr", text);
  bytes += write_model_field(io, im.sumwsq, upstream_name + "_sumwsq", text);
  bytes += write_model_field(io, im.sumwr, upstream_name + "_sumwr", text);
  bytes += write_model_field(io, im.sumw, upstream_name + "_sumw", text);
  bytes += write_model_field(io, im.sumwrxhatlow, upstream_name + "_sumwrxhatlow", text);
  bytes += write_model_field(io, im.sumwxhatlow, upstream_name + "_sumwxhatlow", text);
  bytes += write_model_field(io, im.sumxhatlowsq, upstream_name + "_sumxhatlowsq", text);
  bytes += write_model_field(io, im.sumwrxhathigh, upstream_name + "_sumwrxhathigh", text);
  bytes += write_model_field(io, im.sumwxhathigh, upstream_name + "_sumwxhathigh", text);
  bytes += write_model_field(io, im.sumxhathighsq, upstream_name + "_sumxhathighsq", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW