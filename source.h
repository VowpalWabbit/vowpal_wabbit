/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SCE
#define SCE

#include "io.h"
#include "static_data.h"

struct example_source 
{ 
  //the source of example information.
  io_buf binary;
  io_buf text;
  bool write_cache;
  
  static_data* global;
};

//source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(size_t numbits, example_source& source);
void finalize_source(example_source& source);

void stdin_source(example_source& source, size_t numbits);
void stdin_source(example_source& source, size_t numbits, bool quiet);
void file_source(example_source& source, size_t numbits, string data_file_name);
void file_source(example_source& source, size_t numbits, string data_file_name, bool quiet);
void cache_source(example_source& source, size_t numbits, string cache_file_name);
void cache_source(example_source& source, size_t numbits, string cache_file_name,bool quiet);

#endif
