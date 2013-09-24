/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef SL_H
#define SL_H

#include "io_buf.h"
#include "parse_primitives.h"
#include "global_data.h"
#include "example.h"

struct label_data {
  float label;
  float weight;
  float initial;
};

void return_simple_example(vw& all, example* ec);

size_t read_cached_simple_label(shared_data* sd, void* v, io_buf& cache);
void cache_simple_label(void* v, io_buf& cache);
void default_simple_label(void* v);
void parse_simple_label(parser* p, shared_data* sd, void* v, v_array<substring>& words);
void delete_simple_label(void* v);
float get_weight(void* v);
float get_initial(void* v);
const label_parser simple_label = {default_simple_label, parse_simple_label, 
				   cache_simple_label, read_cached_simple_label, 
				   delete_simple_label, get_weight, get_initial, 
                                   NULL,
				   sizeof(label_data)};

float query_decision(vw& all, example* ec, float k);
bool summarize_holdout_set(vw& all, size_t& no_win_counter);
void print_update(vw& all, example *ec);

#endif
