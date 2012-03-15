#ifndef ECT_H
#define ECT_H

#include "oaa.h"

namespace ECT
{
  void parse_flags(size_t s, size_t e, void (*base_l)(example*), void (*base_f)());
  void learn(example* ec);
  void finish();  
}

#endif
