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
  bool   initialize(vw&,std::vector<std::string>&, po::variables_map&);
  size_t hash(state);
  bool   equivalent(state, state);
  std::string to_string(state, bool, std::vector<action>);
  void cs_ldf_example(vw&, state, action, example*&, bool);
  bool allowed(state, action);
}

#endif
