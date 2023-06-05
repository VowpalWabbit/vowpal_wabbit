// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metrics_collector.h"

#include "vw/core/learner.h"
namespace VW
{
metrics_collector::metrics_collector(bool enabled) : _are_metrics_enabled(enabled) {}

bool metrics_collector::are_metrics_enabled() const { return _are_metrics_enabled; }

void metrics_collector::register_metrics_callback(const metrics_callback_fn& callback)
{
  if (_are_metrics_enabled) { _metrics_callbacks.push_back(callback); }
}

metric_sink metrics_collector::collect_metrics(LEARNER::learner* l) const
{
  VW::metric_sink sink;
  if (!_are_metrics_enabled) { THROW("Metrics must be enabled to call collect_metrics"); }
  if (l) { l->persist_metrics(sink); }
  for (const auto& callback : _metrics_callbacks) { callback(sink); }
  return sink;
}
}  // namespace VW