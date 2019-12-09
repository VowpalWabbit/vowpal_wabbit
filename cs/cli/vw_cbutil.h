// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"

namespace VW
{
public ref class VowpalWabbitContextualBanditUtil abstract sealed
{
public:
  static float GetUnbiasedCost(uint32_t actionObservered, uint32_t actionTaken, float cost, float probability);
};
}
