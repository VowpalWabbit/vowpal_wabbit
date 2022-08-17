// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/io_buf.h"

#include <algorithm>

constexpr float DEFAULT_ALPHA = 0.05f;

namespace VW
{
namespace confidence_sequence
{
struct IncrementalFsum
{
  std::vector<double> partials;

  IncrementalFsum operator+=(double x)
  {
    int i = 0;
    for (double y : this->partials)
    {
      if (std::abs(x) < std::abs(y)) { std::swap(x, y); }
      double hi = x + y;
      double lo = y - (hi - x);
      if (lo != 0.0)
      {
        this->partials[i] = lo;
        ++i;
      }
      x = hi;
    }
    this->partials[i] = x;
    this->partials.resize(i + 1);
    return *this;
  }

  IncrementalFsum operator+(IncrementalFsum const& other)
  {
    IncrementalFsum result;
    result.partials = this->partials;
    for (double y : other.partials) { result += y; }
    return result;
  }

  operator double() const
  {
    double result = 0.0;
    for (double x : this->partials) { result += x; }
    return result;
  }
};

struct IntervalImpl
{
  double eta = 1.1;
  double s = 1.1;
  double rmin = 0.0;
  double rmax = 1.0;
  bool adjust = true;

  int t = 0;

  IncrementalFsum sumwsqrsq;
  IncrementalFsum sumwsqr;
  IncrementalFsum sumwsq;
  IncrementalFsum sumwr;
  IncrementalFsum sumw;
  IncrementalFsum sumwrxhatlow;
  IncrementalFsum sumwxhatlow;
  IncrementalFsum sumxhatlowsq;
  IncrementalFsum sumwrxhathigh;
  IncrementalFsum sumwxhathigh;
  IncrementalFsum sumxhathighsq;

  double reimann_zeta(double n, double z);
  double polygamma(double a, double b);
  double lb_new(double sumXt, double v, double eta, double s, double alpha);
  IntervalImpl(double rmin, double rmax, bool adjust);
  void addobs(double w, double r, double p_drop, double n_drop);
  std::pair<double, double> getci(double alpha = DEFAULT_ALPHA);
};
}  // namespace confidence_sequence

namespace model_utils
{
size_t read_model_field(io_buf&, VW::confidence_sequence::IncrementalFsum&);
size_t write_model_field(io_buf&, const VW::confidence_sequence::IncrementalFsum&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::confidence_sequence::IntervalImpl&);
size_t write_model_field(io_buf&, const VW::confidence_sequence::IntervalImpl&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
