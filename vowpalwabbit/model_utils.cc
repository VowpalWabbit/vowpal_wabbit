// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "model_utils.h"

namespace VW
{
namespace model_utils
{

size_t write_model_field(io_buf& io, const size_t& var, const std::string& upstream_name, bool text)
{
  uint32_t v = static_cast<uint32_t>(var);
  return write_model_field(io, v, upstream_name, text);
}

size_t read_model_field(io_buf& io, size_t& var)
{
  size_t bytes = 0;
  uint32_t v;
  bytes += read_model_field(io, v);
  var = static_cast<size_t>(v);
  return bytes;
}

}  // namespace model_utils
}  // namespace VW