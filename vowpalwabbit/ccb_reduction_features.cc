// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "ccb_reduction_features.h"
#include "model_utils.h"

const char* VW::to_string(CCB::example_type ex_type)
{
#define CASE(type) \
  case type:       \
    return #type;

  using namespace CCB;
  switch (ex_type)
  {
    CASE(example_type::unset)
    CASE(example_type::shared)
    CASE(example_type::action)
    CASE(example_type::slot)
  }

  // The above enum is exhaustive and will warn on a new label type being added due to the lack of `default`
  // The following is required by the compiler, otherwise it things control can reach the end of this function without
  // returning.
  assert(false);
  return "unknown example_type enum";

#undef CASE
}

namespace VW
{
namespace model_utils
{
size_t read_model_field(io_buf& io, CCB::reduction_features& rf)
{
  size_t bytes = 0;
  bytes += read_model_field(io, rf.type);
  bytes += read_model_field(io, rf.explicit_included_actions);
  return bytes;
}
size_t write_model_field(io_buf& io, const CCB::reduction_features& rf, const std::string& upstream_name, bool text)
{
  size_t bytes = 0;
  bytes += write_model_field(io, rf.type, upstream_name + "_type", text);
  bytes += write_model_field(io, rf.explicit_included_actions, upstream_name + "_explicit_included_actions", text);
  return bytes;
}
}  // namespace model_utils
}  // namespace VW
