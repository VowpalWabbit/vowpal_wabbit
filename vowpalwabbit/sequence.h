#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "oaa.h"

<<<<<<< HEAD
typedef uint32_t* history;  // histories have the most recent prediction at the END

struct history_item {
  history  predictions;
  uint32_t predictions_hash;
  float    loss;
  bool     same;
};

void parse_sequence_args(po::variables_map& vm, example* (**gf)(), void (**rf)(example*));
void drive_sequence();

=======
void parse_sequence_args(po::variables_map& vm);
>>>>>>> 8e9e53ef8f1f544d9c1078af5483a705b030bfdb

#endif
