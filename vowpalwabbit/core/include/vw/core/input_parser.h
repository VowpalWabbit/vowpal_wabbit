// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/common/string_view.h"
#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

#include <string>

namespace VW
{
namespace details
{
// Experimental.
class input_parser
{
public:
  input_parser(std::string name) : _name(std::move(name)) {}
  virtual ~input_parser() = default;

  virtual bool next(VW::workspace& workspace_instance, io_buf& buffer, VW::multi_ex& output_examples) = 0;
  VW::string_view get_name() const { return _name; }

private:
  std::string _name;
};

}  // namespace details
}  // namespace VW
