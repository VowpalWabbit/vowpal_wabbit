// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_cbutil.h"
#include "cb_algs.h"

namespace VW
{
float VowpalWabbitContextualBanditUtil::GetUnbiasedCost(uint32_t actionObservered, uint32_t actionTaken, float cost, float probability)
{ CB::cb_class observation = { cost, actionObservered, probability };

  return CB_ALGS::get_cost_estimate(&observation, actionTaken);
}
}
