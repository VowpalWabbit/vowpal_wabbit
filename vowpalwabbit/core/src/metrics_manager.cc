// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/metrics_manager.h"

#include "vw/core/learner.h"
namespace VW
{
metrics_manager::metrics_manager(bool enabled, std::string filename)
    : _are_metrics_enabled(enabled), _metrics_filename(filename)
{
}

bool metrics_manager::are_metrics_enabled() const { return _are_metrics_enabled; }

std::string metrics_manager::get_filename() const { return _metrics_filename; }

void metrics_manager::register_metrics_callback(const metrics_callback_fn& callback)
{
  if (_are_metrics_enabled) { _metrics_callbacks.push_back(callback); }
}

metric_sink metrics_manager::collect_metrics(LEARNER::base_learner* l) const
{
  VW::metric_sink sink;
  if (!_are_metrics_enabled) { THROW("Metrics must be enabled to call collect_metrics"); }
  if (l) { l->persist_metrics(sink); }
  for (const auto& callback : _metrics_callbacks) { callback(sink); }
  return sink;
}
}  // namespace VW