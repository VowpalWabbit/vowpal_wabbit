#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

namespace WAP {
  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)());
  void learn(example* ec);
  void finish();

}
#endif
