#ifndef EX_H
#define EX_H

#include <stdint.h>
#include "v_array.h"

struct label_data {
  double label;
  float weight;
  bool undo;
  v_array<char> tag;
};

struct feature {
  float x;
  uint32_t weight_index;
  bool operator==(feature j){return weight_index == j.weight_index;}
};

struct audit_data {
  char* space;
  char* feature;
  size_t weight_index;
  float x;
  bool alloced;
};

struct example // core example datatype.
{
  void* ld;

  v_array<size_t> indices;
  v_array<feature> atomics[256]; // raw parsed data
  
  v_array<audit_data> audit_features[256];
  
  v_array<feature*> subsets[256];// helper for fast example expansion
  size_t num_features;//precomputed, cause it's fast&easy.
  float partial_prediction;//shared data for prediction.
  float eta_round;

  pthread_mutex_t lock; //thread coordination devices
  size_t threads_to_finish;
  bool in_use; //in use or not (for the parser)
  bool done; //set to false by setup_example()
};

struct partial_example { //used by the multisource parser
  int example_number;
  size_t counter;
  label_data ld;
  v_array<feature> features;
};

#endif
