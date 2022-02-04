// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "config/option.h"
#include "future_compat.h"

#include <string>

namespace VW
{
namespace config
{
struct options_serializer_i
{
  virtual void add(base_option& argument) = 0;
  VW_ATTR(nodiscard) virtual std::string str() const = 0;
  VW_ATTR(nodiscard) virtual size_t size() const = 0;
};

}  // namespace config
}  // namespace VW
