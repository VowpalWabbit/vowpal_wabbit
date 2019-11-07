#pragma once

struct vw_dbg
{
  const static bool default_log     = false;
  const static bool learner         = true;
  const static bool search          = false;
  const static bool gd              = true;
  const static bool gd_predict      = true;
  const static bool binary          = false;
  const static bool cb_adf          = false;
  const static bool csoaa           = true;

  const static bool track_stack     = default_log | learner | search | gd | gd_predict | binary | cb_adf | csoaa;
};

#define VW_DEBUG_LOG vw_dbg::default_log

#define VW_LOG_SINK std::cout

#define VW_DBG(e)   \
  if (VW_DEBUG_LOG) \
  VW_LOG_SINK << depth_indent_string(e)

#define VW_DBG_0    \
  if (VW_DEBUG_LOG) \
  VW_LOG_SINK
