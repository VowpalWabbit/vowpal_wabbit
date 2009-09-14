#ifndef SL_H
#define SL_H

#include "io.h"
#include "parse_primitives.h"
#include "parser.h"

size_t read_cached_simple_label(void* v, io_buf& cache);
void cache_simple_label(void* v, io_buf& cache);
void default_simple_label(void* v);
void parse_simple_label(void* v, substring label_space, v_array<substring>& words);
void delete_simple_label(void* v);
const label_parser simple_label = {default_simple_label, parse_simple_label, 
				   cache_simple_label, read_cached_simple_label, 
				   delete_simple_label, sizeof(label_data)};

#endif
