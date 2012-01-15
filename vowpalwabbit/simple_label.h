#ifndef SL_H
#define SL_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

struct label_data {
  float label;
  float weight;
  float initial;
};

void return_simple_example(example* ec);
example* get_simple_example();

size_t read_cached_simple_label(void* v, io_buf& cache);
void cache_simple_label(void* v, io_buf& cache);
void default_simple_label(void* v);
void parse_simple_label(void* v, v_array<substring>& words);
void delete_simple_label(void* v);
float get_weight(void* v);
float get_initial(void* v);
const label_parser simple_label = {default_simple_label, parse_simple_label, 
				   cache_simple_label, read_cached_simple_label, 
				   delete_simple_label, get_weight, get_initial, 
				   sizeof(label_data)};

#endif
