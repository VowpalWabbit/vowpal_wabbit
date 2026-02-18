#include "vw.net.cbutil.h"

API float GetCbUnbiasedCost(uint32_t actionObservered, uint32_t actionTaken, float cost, float probability)
{
  VW::cb_class observation(cost, actionObservered, probability);
  return VW::get_cost_estimate(observation, actionTaken);
}