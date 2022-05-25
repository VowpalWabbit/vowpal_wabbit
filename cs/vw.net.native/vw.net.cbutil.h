#pragma once

#include "vw.net.native.h"
#include "cb.h"
#include "cb_algs.h"

extern "C" {
  API float GetCbUnbiasedCost(uint32_t actionObservered, uint32_t actionTaken, float cost, float probability);
}
