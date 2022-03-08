// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "ccb_reduction_features.h"
#include "continuous_actions_reduction_features.h"
#include "epsilon_reduction_features.h"
#include "simple_label.h"

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
  simple_label_reduction_features _simple_label_reduction_features;
  VW::cb_explore_adf::greedy::reduction_features _epsilon_reduction_features;

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
    _simple_label_reduction_features.reset_to_default();
    _epsilon_reduction_features.reset_to_default();
  }
};

template <>
inline CCB::reduction_features& reduction_features::get<CCB::reduction_features>()
{
  return _ccb_reduction_features;
}

template <>
inline const CCB::reduction_features& reduction_features::get<CCB::reduction_features>() const
{
  return _ccb_reduction_features;
}

template <>
inline VW::continuous_actions::reduction_features& reduction_features::get<VW::continuous_actions::reduction_features>()
{
  return _contact_reduction_features;
}

template <>
inline const VW::continuous_actions::reduction_features&
reduction_features::get<VW::continuous_actions::reduction_features>() const
{
  return _contact_reduction_features;
}

template <>
inline simple_label_reduction_features& reduction_features::get<simple_label_reduction_features>()
{
  return _simple_label_reduction_features;
}

template <>
inline const simple_label_reduction_features& reduction_features::get<simple_label_reduction_features>() const
{
  return _simple_label_reduction_features;
}

template <>
inline VW::cb_explore_adf::greedy::reduction_features&
reduction_features::get<VW::cb_explore_adf::greedy::reduction_features>()
{
  return _epsilon_reduction_features;
}

template <>
inline const VW::cb_explore_adf::greedy::reduction_features&
reduction_features::get<VW::cb_explore_adf::greedy::reduction_features>() const
{
  return _epsilon_reduction_features;
}
