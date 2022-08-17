// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#ifndef __APPLE__
#  define __STDCPP_MATH_SPEC_FUNCS__ 201003L
#  define __STDCPP_WANT_MATH_SPEC_FUNCS__ 1
#endif

#include "vw/core/io_buf.h"
#include "vw/core/metric_sink.h"

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

struct ConfidenceSequence
{
  double alpha;
  double rmin_init;
  double rmin;
  double rmax_init;
  double rmax;
  bool adjust;

  double eta;
  double s;
  int t;

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

  uint64_t update_count;
  double last_w;
  double last_r;

public:
  ConfidenceSequence(double alpha = DEFAULT_ALPHA, double rmin_init = 0.0, double rmax_init = 1.0, bool adjust = true);
  void update(double w, double r, double p_drop = 0.0, double n_drop = -1.0);
  void persist(metric_sink&, const std::string&);
  void reset_stats();
  float lower_bound();
  float upper_bound();

private:
  double approxpolygammaone(double b);
  double lblogwealth(double sumXt, double v, double eta, double s, double lb_alpha);
};
}  // namespace confidence_sequence

namespace model_utils
{
size_t read_model_field(io_buf&, VW::confidence_sequence::IncrementalFsum&);
size_t write_model_field(io_buf&, const VW::confidence_sequence::IncrementalFsum&, const std::string&, bool);
size_t read_model_field(io_buf&, VW::confidence_sequence::ConfidenceSequence&);
size_t write_model_field(io_buf&, const VW::confidence_sequence::ConfidenceSequence&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
