#pragma once

#include "vw.net.native.h"
#include "vw/core/vw.h"

extern "C" {
  API multi_ex* CreateMultiEx();
  API void DeleteMultiEx(multi_ex* multi_ex);

  API void MultiExAddExample(multi_ex* multi_ex, example* ex);
}
