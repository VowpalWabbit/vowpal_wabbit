#pragma once

#include "vw.net.native.h"
#include "vw/core/vw.h"

extern "C"
{
  API VW::multi_ex* CreateMultiEx();
  API void DeleteMultiEx(VW::multi_ex* multi_ex);

  API void MultiExAddExample(VW::multi_ex* multi_ex, VW::example* ex);
}
