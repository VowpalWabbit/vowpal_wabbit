// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/multi_ex.h"
#include "vw/core/vw_fwd.h"

namespace VW
{
namespace LEARNER
{
template <class T, class E>
class learner;
using base_learner = learner<char, char>;
using single_learner = learner<char, example>;
using multi_learner = learner<char, multi_ex>;
}  // namespace LEARNER
}  // namespace VW
