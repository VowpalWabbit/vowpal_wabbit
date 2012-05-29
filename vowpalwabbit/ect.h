#ifndef ECT_H
#define ECT_H

#include "oaa.h"

namespace ECT
{
  void parse_flags(vw&, size_t s, size_t e, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void learn(vw& all, example* ec);
  void finish(vw&);  
}

#endif
