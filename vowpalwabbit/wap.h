#ifndef WAP_H
#define WAP_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"
#include "oaa.h"
#include "csoaa.h"

namespace WAP {
  void parse_flags(vw&, std::vector<std::string>&, size_t s, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void learn(vw&,example* ec);
  void finish(vw&);

}

namespace WAP_LDF {
  typedef OAA::mc_label label;
  
  void parse_flags(vw&, std::vector<std::string>&, size_t s, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void global_print_newline();
  void learn(vw&,example* ec);
  void finish(vw&);

  const label_parser cs_label_parser = CSOAA_LDF::cs_label_parser;
}

#endif
