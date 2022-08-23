#pragma once

#include "vw.net.native.h"
#include "vw/core/cb.h"
#include "vw/core/reductions/cb/cb_algs.h"

extern "C"
{
  API float GetCbUnbiasedCost(uint32_t actionObservered, uint32_t actionTaken, float cost, float probability);
}
