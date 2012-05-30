#ifndef SEARN_SEQUENCETASK_H
#define SEARN_SEQUENCETASK_H

#include "oaa.h"
#include "parse_primitives.h"
#include "searn.h"

namespace SequenceTask {
  bool   final(state);
  float  loss(state);
  void   step(state, action);
  action oracle(state);
  state  copy(state);
  void   finish(state);
  void   start_state_multiline(example**, size_t, state*);
  void   cs_example(vw&, state, example*&, bool);
  bool   initialize(po::variables_map& vm);
  size_t hash(state);
  bool   equivalent(state, state);
  //  void   to_string(state, size_t, char*, char*);
}

#endif
