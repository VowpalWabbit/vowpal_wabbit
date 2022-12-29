// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metrics_manager.h"

namespace VW
{
  metrics_manager::metrics_manager(bool enabled, std::string filename) : _are_metrics_enabled(enabled), _metrics_filename(filename) {}

  bool metrics_manager::are_metrics_enabled() const { return  _are_metrics_enabled; }

  std::string metrics_manager::get_filename() const { return _metrics_filename; }

  void metrics_manager::register_metrics_callback(const metrics_callback_fn& callback) { _metrics_callbacks.push_back(callback); }

  metric_sink metrics_manager::collect_metrics()
  {
    VW::metric_sink sink;
    while (!_metrics_callbacks.empty())
    {
      const metrics_callback_fn& callback = _metrics_callbacks.back();
      callback(sink);
      _metrics_callbacks.pop_back();
    }
    return sink;
  }
}  // namespace VW