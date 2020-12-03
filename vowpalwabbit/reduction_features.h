// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "ccb_reduction_features.h"
#include "continuous_actions_reduction_features.h"

/*
 * class reduction_features
 * Description:
 *   This data structure manages access and lifetime of the features data used in various VW reductions.
 *
 *   All contained data types MUST instantiate a `clear()` function that readies the object for reuse.
 *
 *   Calling create() on an existing element will clear and overwrite the existing data
 *
 * Member functions:
 *   T& get<T>();
 *   const T& get<T>() const;
 *   void clear();
 *
 * Data structure usage:
 *   Accessing an instance of existing data:
 *     features_data fd;
 *     auto& data = fd.get<data_type>();
 */

class reduction_features
{
private:
  CCB::reduction_features _ccb_reduction_features;
  VW::continuous_actions::reduction_features _contact_reduction_features;

public:
  template <typename T>
  T& get();
  template <typename T>
  const T& get() const;

  // call clear() on all instantiated types
  void clear()
  {
    _ccb_reduction_features.clear();
    _contact_reduction_features.clear();
  }
};

template <>
CCB::reduction_features& reduction_features::get<CCB::reduction_features>();
template <>

const CCB::reduction_features& reduction_features::get<CCB::reduction_features>() const;

template <>
VW::continuous_actions::reduction_features& reduction_features::get<VW::continuous_actions::reduction_features>();

template <>
const VW::continuous_actions::reduction_features& reduction_features::get<VW::continuous_actions::reduction_features>()
    const;