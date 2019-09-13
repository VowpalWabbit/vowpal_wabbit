#pragma once

#define VW_DEBUG_ENABLE(x) static bool VW_DEBUG_LOG = x;

#define VW_DBG(e)   \
  if (VW_DEBUG_LOG) \
  std::cout << depth_indent_string(e)
