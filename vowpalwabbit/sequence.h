#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"

namespace Sequence {
  void parse_flags(vw&, std::vector<std::string>&, po::variables_map&, void (*base_l)(vw&, example*), void (*base_f)(vw&));
  void drive(void*);
}

#endif
