/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef SCE
#define SCE


#include "io.h"
#include "global_data.h"
#include "parse_primitives.h"
#include "example.h"

struct label_parser {
  void (*default_label)(void*);
  void (*parse_label)(void*, v_array<substring>&);
  void (*cache_label)(void*, io_buf& cache);
  size_t (*read_cached_label)(void*, io_buf& cache);
  void (*delete_label)(void*);
  float (*get_weight)(void*);
  float (*get_initial)(void*);
  size_t label_size;
};

typedef size_t (*hash_func_t)(substring, unsigned long);

struct parser {
  v_array<substring> channels;//helper(s) for text parsing
  v_array<substring> words;
  v_array<substring> name;

  const label_parser* lp;
  float t;

  io_buf* input; //Input source(s)
  int (*reader)(parser* p, void* ae);
  hash_func_t hasher;
  bool resettable; //Whether or not the input can be reset.
  io_buf* output; //Where to output the cache.
  bool write_cache; 
  bool sort_features;
  bool sorted_cache;

  v_array<partial_example> pes;//partial examples
  v_array<size_t> ids; //unique ids for sources
  v_array<size_t> counts; //partial examples received from sources
  size_t finished_count;//the number of finished examples;
  int label_sock;
  int bound_sock;
  int max_fd;
};

const size_t constant_namespace = 128;

parser* new_parser(const label_parser* lp);
#include <boost/program_options.hpp>
namespace po = boost::program_options;
void parse_source_args(po::variables_map& vm, parser* par, bool quiet, size_t passes);

bool examples_to_finish();

//parser control

void start_parser(size_t num_threads, parser* pf);
void end_parser(parser* pf);
example* get_example(size_t thread_num);
void free_example(example* ec);
void make_example_available();
bool parser_done();

//source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(size_t numbits, parser* source);
void finalize_source(parser* source);
void set_compressed(parser* par);

//NGrma functions
void generateGrams(size_t ngram, size_t skip_gram, example * &ex);
void generateGrams(size_t ngram, size_t skip_gram, v_array<feature> &atomics, size_t* indices);

#endif
