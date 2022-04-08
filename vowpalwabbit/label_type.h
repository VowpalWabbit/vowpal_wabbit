// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw_string_view.h"

#include <cstdint>

namespace VW
{
enum class label_type_t : uint32_t
{
  simple,
  cb,       // contextual-bandit
  cb_eval,  // contextual-bandit evaluation
  cs,       // cost-sensitive
  multilabel,
  multiclass,
  ccb,  // conditional contextual-bandit
  slates,
  nolabel,
  continuous  // continuous actions
};
string_view to_string(VW::label_type_t);
}  // namespace VW

using label_type_t VW_DEPRECATED(
    "Global namespace label_type_t is deprecated. Use VW::label_type_t.") = VW::label_type_t;
