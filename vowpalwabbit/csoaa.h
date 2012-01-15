#ifndef CSOAA_H
#define CSOAA_H

#include "io.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

struct csoaa_data {
  v_array<float> costs;
};

void parse_csoaa_flag(size_t s);
void return_csoaa_example(example* ec);
example* get_csoaa_example();

size_t read_cached_csoaa_label(void* v, io_buf& cache);
void cache_csoaa_label(void* v, io_buf& cache);
void default_csoaa_label(void* v);
void parse_csoaa_label(void* v, v_array<substring>& words);
void delete_csoaa_label(void* v);
float csoaa_weight(void* v);
float csoaa_initial(void* v);
const label_parser csoaa_label = {default_csoaa_label, parse_csoaa_label, 
				cache_csoaa_label, read_cached_csoaa_label, 
				delete_csoaa_label, get_weight, get_initial, 
				sizeof(csoaa_data)};

#endif
