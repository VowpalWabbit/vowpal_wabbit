/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PE_H
#define PE_H
#include "io.h"
#include "parse_regressor.h"
#include "parse_primitives.h"
#include "source.h"

using namespace std;

struct feature {
  float x;
  uint32_t weight_index;
  bool operator==(feature j){return weight_index == j.weight_index;}
};

struct label_parser {
  void (*default_label)(void*);
  void (*parse_label)(void*, substring, v_array<substring>&);
  void (*cache_label)(void*, io_buf& cache);
  size_t (*read_cached_label)(void*, io_buf& cache);
  void (*delete_label)(void*);
  size_t label_size;
};

struct parser {
  v_array<substring> channels;
  v_array<substring> words;
  v_array<substring> name;

  example_source* source;

  const label_parser* lp;
};

parser* new_parser(example_source* s, const label_parser* lp);

struct audit_data {
  char* space;
  char* feature;
  size_t weight_index;
  float x;
  bool alloced;
};

struct example
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

//parser control

void setup_parser(size_t num_threads, parser* pf);
void destroy_parser(parser* pf);
//example processing
example* get_example(example* ec, size_t thread_num);

#endif
