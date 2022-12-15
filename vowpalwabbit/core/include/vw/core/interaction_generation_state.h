// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/vw_exception.h"
#include "vw/core/constant.h"
#include "vw/core/example_predict.h"
#include "vw/core/feature_group.h"
#include "vw/core/object_pool.h"

#include <vector>

namespace VW
{
namespace details
{
// state data used in non-recursive feature generation algorithm
// contains N feature_gen_data records (where N is length of interaction)
class feature_gen_data
{
public:
  uint64_t hash = 0;              // hash of feature interactions of previous namespaces in the list
  float x = 1.f;                  // value of feature interactions of previous namespaces in the list
                                  // than once calculated at preprocessing together with same_ns
  bool self_interaction = false;  // namespace interacting with itself
  features::const_audit_iterator begin_it;
  features::const_audit_iterator current_it;
  features::const_audit_iterator end_it;

  feature_gen_data(features::const_audit_iterator begin, features::const_audit_iterator end)
      : begin_it(begin), current_it(begin), end_it(end)
  {
  }
};

using features_range_t = std::pair<features::const_audit_iterator, features::const_audit_iterator>;

class extent_interaction_expansion_stack_item
{
public:
  size_t current_term;
  size_t prev_term;
  size_t offset;
  std::vector<features_range_t> so_far;
};

class generate_interactions_object_cache
{
public:
  std::vector<feature_gen_data> state_data;
  VW::moved_object_pool<extent_interaction_expansion_stack_item> frame_pool;
  std::stack<extent_interaction_expansion_stack_item> in_process_frames;
};
}  // namespace details
}  // namespace VW
