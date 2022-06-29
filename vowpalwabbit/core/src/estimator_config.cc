// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/estimator_config.h"

#include "vw/core/model_utils.h"

namespace VW
{
void estimator_config::update(float w, float r)
{
  update_count++;
  chisq.update(w, r);
  ips += r * w;
  last_w = w;
  last_r = r;
}

void estimator_config::persist(metric_sink& metrics, const std::string& suffix)
{
  metrics.set_uint("upcnt" + suffix, update_count);
  metrics.set_float("ips" + suffix, current_ips());
  distributionally_robust::ScoredDual sd = chisq.recompute_duals();
  metrics.set_float("bound" + suffix, static_cast<float>(sd.first));
  metrics.set_float("w" + suffix, last_w);
  metrics.set_float("r" + suffix, last_r);
}

float estimator_config::current_ips() const { return (update_count > 0) ? ips / update_count : 0; }

void estimator_config::reset_stats(double alpha, double tau)
{
  chisq.reset(alpha, tau);
  ips = 0.0;
  last_w = 0.0;
  last_r = 0.0;
  update_count = 0;
}

float estimator_config::lower_bound() { return chisq.cressieread_lower_bound(); }

float estimator_config::upper_bound() { return chisq.cressieread_upper_bound(); }

namespace model_utils
{
size_t read_model_field(io_buf& io, VW::estimator_config& sc)
{
  size_t bytes = 0;
  bytes += read_model_field(io, sc.chisq);
  bytes += read_model_field(io, sc.ips);
  bytes += read_model_field(io, sc.update_count);
  bytes += read_model_field(io, sc.last_w);
  bytes += read_model_field(io, sc.last_r);
  return bytes;
}

size_t write_model_field(io_buf& io, const VW::estimator_config& sc, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, sc.chisq, upstream_name + "_chisq", text);
  bytes += write_model_field(io, sc.ips, upstream_name + "_ips", text);
  bytes += write_model_field(io, sc.update_count, upstream_name + "_count", text);
  bytes += write_model_field(io, sc.last_w, upstream_name + "_lastw", text);
  bytes += write_model_field(io, sc.last_r, upstream_name + "_lastr", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW