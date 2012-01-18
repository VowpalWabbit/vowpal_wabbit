#ifndef EX_H
#define EX_H

#include <stdint.h>
#include <pthread.h>
#include "v_array.h"

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

typedef float simple_prediction;

struct example // core example datatype.
{
  void* ld;
  simple_prediction final_prediction;

  v_array<char> tag;//An identifier for the example.
  size_t example_counter;

  v_array<size_t> indices;
  v_array<feature> atomics[256]; // raw parsed data
  
  v_array<audit_data> audit_features[256];
  
  size_t num_features;//precomputed, cause it's fast&easy.
  size_t pass;
  float partial_prediction;//shared data for prediction.
  v_array<float> topic_predictions;
  float loss;
  float eta_round;
  float eta_global;
  float global_weight;
  float example_t;//sum of importance weights so far.
  float sum_feat_sq[256];//helper for total_sum_feat_sq.
  float total_sum_feat_sq;//precomputed, cause it's kind of fast & easy.
  float revert_weight;

  bool sorted;//Are the features sorted or not?
  bool in_use; //in use or not (for the parser)
  bool done; //set to false by setup_example()
};

#endif
