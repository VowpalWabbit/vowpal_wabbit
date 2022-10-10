#include "typesys.h"

namespace typesys
{
universe& universe::instance()
{
  static universe instance;
  return instance;
}

universe::universe()
{
  
}
}