// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/example.h"

#include <unordered_map>

namespace VW
{
namespace details
{
using label_feature_map = std::unordered_map<size_t, features>;
void append_example_namespace_from_memory(const label_feature_map& lfm, VW::example& ec, size_t lab);
void truncate_example_namespace_from_memory(const label_feature_map& lfm, VW::example& ec, size_t lab);
}  // namespace details
}  // namespace VW
