// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

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

template <>
VW::continuous_actions::reduction_features& reduction_features::get<VW::continuous_actions::reduction_features>()
{
  return _contact_reduction_features;
}
template <>
const VW::continuous_actions::reduction_features& reduction_features::get<VW::continuous_actions::reduction_features>() const
{
  return _contact_reduction_features;
}
