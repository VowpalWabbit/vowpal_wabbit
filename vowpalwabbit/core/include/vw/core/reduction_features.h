// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/cb_graph_feedback_reduction_features.h"
#include "vw/core/ccb_reduction_features.h"
#include "vw/core/continuous_actions_reduction_features.h"
#include "vw/core/epsilon_reduction_features.h"
#include "vw/core/large_action_space_reduction_features.h"
#include "vw/core/simple_label.h"

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

namespace VW
{
class reduction_features
{
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
    _large_action_space_reduction_features.reset_to_default();
    _cb_graph_feedback_reduction_features.clear();
  }

private:
  VW::ccb_reduction_features _ccb_reduction_features;
  VW::continuous_actions::reduction_features _contact_reduction_features;
  simple_label_reduction_features _simple_label_reduction_features;
  VW::cb_explore_adf::greedy::reduction_features _epsilon_reduction_features;
  VW::large_action_space::las_reduction_features _large_action_space_reduction_features;
  VW::cb_graph_feedback::reduction_features _cb_graph_feedback_reduction_features;
};

template <>
inline VW::ccb_reduction_features& reduction_features::get<VW::ccb_reduction_features>()
{
  return _ccb_reduction_features;
}

template <>
inline const VW::ccb_reduction_features& reduction_features::get<VW::ccb_reduction_features>() const
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
inline simple_label_reduction_features& reduction_features::get<VW::simple_label_reduction_features>()
{
  return _simple_label_reduction_features;
}

template <>
inline const simple_label_reduction_features& reduction_features::get<VW::simple_label_reduction_features>() const
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

template <>
inline VW::large_action_space::las_reduction_features&
reduction_features::get<VW::large_action_space::las_reduction_features>()
{
  return _large_action_space_reduction_features;
}

template <>
inline const VW::large_action_space::las_reduction_features&
reduction_features::get<VW::large_action_space::las_reduction_features>() const
{
  return _large_action_space_reduction_features;
}

template <>
inline VW::cb_graph_feedback::reduction_features& reduction_features::get<VW::cb_graph_feedback::reduction_features>()
{
  return _cb_graph_feedback_reduction_features;
}

template <>
inline const VW::cb_graph_feedback::reduction_features&
reduction_features::get<VW::cb_graph_feedback::reduction_features>() const
{
  return _cb_graph_feedback_reduction_features;
}
}  // namespace VW

using reduction_features VW_DEPRECATED("reduction_features moved into VW namespace") = VW::reduction_features;