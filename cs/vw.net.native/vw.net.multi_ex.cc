#include "vw.net.multi_ex.h"

API multi_ex* CreateMultiEx()
{
  return new multi_ex;
}

API void DeleteMultiEx(multi_ex* multi_ex)
{
  delete multi_ex;
}

API void MultiExAddExample(multi_ex* multi_ex, example* ex)
{
  multi_ex->push_back(ex);
}
