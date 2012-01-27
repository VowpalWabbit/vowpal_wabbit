#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "csoaa.h"

namespace WAP {
  
  void parse_flag(size_t s);
  
}

namespace WAP_LDF {
  typedef OAA::mc_label label;
  
  void parse_flag(size_t s);
  void global_print_newline();

  const label_parser cs_label_parser = CSOAA_LDF::cs_label_parser;
}

#endif
