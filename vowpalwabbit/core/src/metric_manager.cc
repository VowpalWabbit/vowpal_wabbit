// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metric_manager.h"

namespace VW
{
  metric_manager::metric_manager(bool enabled) : _are_metrics_enabled(enabled) {}

  bool metric_manager::are_metrics_enabled() const { return  _are_metrics_enabled; }

  void metric_manager::register_metric_callback(const metrics_callback_fn& callback) { _metric_callbacks.push_back(callback); }

  metric_sink metric_manager::collect_metrics()
  {
    VW::metric_sink sink;
    for (auto& callback : _metric_callbacks)
    {
      callback(sink);
    }
    return sink;
  }
}  // namespace VW