#pragma once

#define VW_DEBUG_LOG true

#define VW_DBG(e)   \
  if (VW_DEBUG_LOG) \
  std::cout << depth_indent_string(e)
