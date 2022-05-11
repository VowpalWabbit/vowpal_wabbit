// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/vw_fwd.h"

#include <string>

namespace VW
{
namespace details
{
// Experimental.
struct input_parser
{
  input_parser(std::string name) : _name(std::move(name)) {}

  virtual bool next(VW::workspace& workspace_instance, io_buf& buffer, VW::v_array<VW::example*>& output_examples) = 0;
  std::string_view get_name() const { return _name; }

private:
  std::string _name;
};

}  // namespace details
}  // namespace VW
