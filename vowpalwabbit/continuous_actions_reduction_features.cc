// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "continuous_actions_reduction_features.h"
#include "model_utils.h"

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, VW::continuous_actions::reduction_features& carf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, carf.pdf);
  bytes += read_model_field(io, carf.chosen_action);
  return bytes;
}
size_t write_model_field(io_buf& io, const VW::continuous_actions::reduction_features& carf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, carf.pdf, upstream_name + "_pdf", text);
  bytes += write_model_field(io, carf.chosen_action, upstream_name + "_chosen_action", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW