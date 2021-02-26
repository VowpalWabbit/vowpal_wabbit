// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#define VW_DEBUG_ENABLE(x) static bool VW_DEBUG_LOG = x;

// TODO: Should this be moved to the io library and made to use the logger?
//       we'll need to break the dependency on depth_indent_string() though
#define VW_DBG(e) \
  if (VW_DEBUG_LOG) std::cout << depth_indent_string(e)
