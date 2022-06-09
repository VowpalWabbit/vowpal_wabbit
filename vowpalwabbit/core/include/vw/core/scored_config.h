// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/distributionally_robust.h"
#include "vw/core/metric_sink.h"

namespace VW
{
struct scored_config
{
  VW::distributionally_robust::ChiSquared chisq;
  float ips = 0.0;
  float last_w = 0.0;
  float last_r = 0.0;
  uint64_t update_count = 0;

  scored_config() : chisq(0.05, 0.999, 0, std::numeric_limits<double>::infinity()) {}
  scored_config(double alpha, double tau) : chisq(alpha, tau, 0, std::numeric_limits<double>::infinity()) {}

  void update(float w, float r);
  void persist(metric_sink&, const std::string&);
  float current_ips() const;
  void reset_stats(double alpha = DEFAULT_ALPHA, double tau = CRESSEREAD_DEFAULT_TAU);
};

namespace model_utils
{
size_t read_model_field(io_buf&, VW::scored_config&);
size_t write_model_field(io_buf&, const VW::scored_config&, const std::string&, bool);
}  // namespace model_utils
}  // namespace VW
