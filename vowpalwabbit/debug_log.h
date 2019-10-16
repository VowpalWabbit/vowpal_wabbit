#pragma once

struct vw_dbg
{
  const static bool default     = false;
  const static bool learner     = true;
  const static bool search      = true;
  const static bool gd          = true;
  const static bool gd_predict  = true;
};

#define VW_DEBUG_LOG vw_dbg::default

#define VW_LOG_SINK std::cout

#define VW_DBG(e)   \
  if (VW_DEBUG_LOG) \
  VW_LOG_SINK << depth_indent_string(e)

#define VW_DBG_0    \
  if (VW_DEBUG_LOG) \
  VW_LOG_SINK
