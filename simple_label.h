#ifndef SL_H
#define SL_H

#include "io.h"
#include "parse_primitives.h"
#include "parser.h"

char* bufread_simple_label(label_data* ld, char* c);
char* bufcache_simple_label(label_data* ld, char* c);

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
