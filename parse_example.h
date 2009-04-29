/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef PE_H
#define PE_H
#include "stack.h"
#include "io.h"
#include "parse_regressor.h"

using namespace std;

struct feature {
  float x;
  size_t weight_index;
};

struct substring {
  char *start;
  char *end;
};

struct thread_data {
  size_t linesize;
  char *line;
  
  v_array<substring> channels;
  v_array<substring> words;
  v_array<substring> name;

  v_array<size_t> indicies;
  v_array<feature> atomics[256];
  bool *in_already;
};
  
struct example_file {
  FILE* data;
  io_buf cache;
  bool write_cache;
  size_t mask;
};

// Parse a string 'page_off' to extract features and label.
bool parse_example(thread_data &stuff, example_file &example_source, 
		   regressor &reg, v_array<feature> &features,
		   float &label, float &weight, v_array<char> &tag);

bool inconsistent_cache(size_t numbits, io_buf &cache);

void reset(size_t numbits, example_file &source);

void finalize_source(example_file &source);

#endif
