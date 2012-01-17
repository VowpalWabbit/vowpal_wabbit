#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"

typedef uint32_t* history;  // histories have the most recent prediction at the END

struct history_item {
  history  predictions;
  uint32_t predictions_hash;
  float    loss;
};

void parse_sequence_args(po::variables_map& vm, example* (**gf)(), void (**rf)(example*));

#endif
