// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#define VW_DEBUG_ENABLE(x) static bool VW_DEBUG_LOG = x;

#define VW_DBG(e) \
  if (VW_DEBUG_LOG) std::cout << depth_indent_string(e)
