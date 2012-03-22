#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "csoaa.h"

namespace WAP {
  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)());
  void learn(example* ec);
  void finish();

}

namespace WAP_LDF {
  typedef OAA::mc_label label;
  
  void parse_flags(size_t s, void (*base_l)(example*), void (*base_f)());
  void global_print_newline();
  void learn(example* ec);
  void finish();

  const label_parser cs_label_parser = CSOAA_LDF::cs_label_parser;
}

#endif
