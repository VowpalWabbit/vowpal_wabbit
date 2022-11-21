// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "vw/core/constant.h"
#include "vw/core/v_array.h"
#include "vw/core/vw_fwd.h"

#include <cstdint>
#include <sstream>
#include <string>

namespace VW
{
class active_multiclass_prediction
{
public:
  uint32_t predicted_class = 0;
  v_array<uint32_t> more_info_required_for_classes;
};

inline std::string to_string(const VW::active_multiclass_prediction& active_multiclass)
{
  std::ostringstream ss;
  ss << active_multiclass.predicted_class;
  if (!active_multiclass.more_info_required_for_classes.empty()) { ss << ":"; }

  for (size_t i = 0; i < active_multiclass.more_info_required_for_classes.size(); i++)
  {
    if (i > 0) { ss << ','; }
    ss << active_multiclass.more_info_required_for_classes[i];
  }

  return ss.str();
}

}  // namespace VW