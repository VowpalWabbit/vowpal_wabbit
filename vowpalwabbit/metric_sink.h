#pragma once
// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

namespace VW
{
struct metric_sink
{
  std::vector<std::tuple<std::string, size_t>> int_metrics_list;
  std::vector<std::tuple<std::string, float>> float_metrics_list;
};
}  // namespace VW
