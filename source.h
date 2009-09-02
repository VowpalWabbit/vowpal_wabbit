/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SCE
#define SCE

#include "io.h"
#include "static_data.h"
#include "parse_primitives.h"

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

  const label_parser* lp;

  io_buf input;
  int (*reader)(parser* p, void* ae);
  bool resettable; //whether or not the input can be reset.
  io_buf output;
  bool write_cache;
  
  static_data* global;
};

parser* new_parser(const label_parser* lp);
void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes);

//source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(size_t numbits, parser* source);
void finalize_source(parser* source);

#endif
