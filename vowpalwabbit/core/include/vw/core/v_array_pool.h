// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/object_pool.h"
#include "vw/core/v_array.h"

namespace VW
{
template <typename T>
using v_array_pool = VW::moved_object_pool<v_array<T>>;
template <typename T>
using vector_pool = VW::moved_object_pool<std::vector<T>>;
}  // namespace VW