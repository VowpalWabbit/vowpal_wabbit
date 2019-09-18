#pragma once

#define VW_DEBUG_LOG true

#define VW_LOG_SINK std::cout

#define VW_DBG(e)   \
  if (VW_DEBUG_LOG) \
    VW_LOG_SINK << depth_indent_string(e)

#define VW_DBG_0    \
  if (VW_DEBUG_LOG) \
    VW_LOG_SINK
