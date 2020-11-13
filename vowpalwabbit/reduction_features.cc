#include "reduction_features.h"

template <>
CCB::reduction_features& reduction_features::get<CCB::reduction_features>()
{
  return _ccb_reduction_features;
}
template <>
const CCB::reduction_features& reduction_features::get<CCB::reduction_features>() const
{
  return _ccb_reduction_features;
}
