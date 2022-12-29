// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.=

#pragma once

#include "metric_sink.h"
#include "vw/core/learner_fwd.h"

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace VW
{
class metrics_manager
{
public:
  metrics_manager(bool enabled = false, std::string filename = "");

  using metrics_callback_fn = std::function<void(VW::metric_sink&)>;

  bool are_metrics_enabled() const;
  std::string get_filename() const;
  void register_metrics_callback(const metrics_callback_fn& callback);
  VW::metric_sink collect_metrics(LEARNER::base_learner* l = nullptr) const;

private:
  bool _are_metrics_enabled;
  std::vector<metrics_callback_fn> _metrics_callbacks;
  std::string _metrics_filename;
};
}  // namespace VW