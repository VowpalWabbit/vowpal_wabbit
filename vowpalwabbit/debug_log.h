// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "future_compat.h"
#pragma once

struct vw_dbg
{
  static constexpr bool default_log = false;
  static constexpr bool learner = false;
  static constexpr bool cats_pdf = false;
};

#define VW_DEBUG_LOG vw_dbg::default_log

#define VW_LOG_SINK std::cout

#define VW_DBG(e) \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK << depth_indent_string(e)

#define VW_DBG_0 \
  if VW_STD17_CONSTEXPR (VW_DEBUG_LOG) VW_LOG_SINK
