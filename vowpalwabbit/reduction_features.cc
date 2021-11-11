// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "reduction_features.h"
#include "model_utils.h"

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, reduction_features& rf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, rf._ccb_reduction_features);
  bytes += read_model_field(io, rf._contact_reduction_features);
  bytes += read_model_field(io, rf._simple_label_reduction_features);
  return bytes;
}
size_t write_model_field(io_buf& io, const reduction_features& rf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, rf._ccb_reduction_features, upstream_name + "_ccb_reduction_features", text);
  bytes += write_model_field(io, rf._contact_reduction_features, upstream_name + "_contact_reduction_features", text);
  bytes += write_model_field(io, rf._simple_label_reduction_features, upstream_name + "_simple_label_reduction_features", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW