/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "io_buf.h"
#include "parse_primitives.h"
#include "example.h"

// Mutex and CV cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed
// project.
#ifdef _M_CEE
#pragma managed(push, off)
#undef _M_CEE
#include <mutex>
#include <condition_variable>
#define _M_CEE 001
#pragma managed(pop)
#else
#include <mutex>
#include <condition_variable>
#endif

#include <memory>
#include "queue.h"
#include "object_pool.h"

struct vw;
struct input_options;

struct example_factory
{
  example* operator()()
  {
    auto new_example = new example{};
    memset(&new_example->l, 0, sizeof(polylabel));
    new_example->in_use = false;
    return new_example;
  }
};

struct parser
{
  v_array<substring> channels;  // helper(s) for text parsing
  v_array<substring> words;
  v_array<substring> name;

  std::mutex pool_lock;
  std::unique_ptr<VW::unbounded_object_pool<example, example_factory>> example_pool;
  std::unique_ptr<VW::ptr_queue<example>> ready_parsed_examples;

  io_buf* input;  // Input source(s)
  int (*reader)(vw*, v_array<example*>& examples);
  hash_func_t hasher;
  bool resettable;  // Whether or not the input can be reset.
  io_buf* output;   // Where to output the cache.
  bool write_cache;
  bool sort_features;
  bool sorted_cache;

  size_t ring_size;
  uint64_t begin_parsed_examples;  // The index of the beginning parsed example.
  uint64_t end_parsed_examples;    // The index of the fully parsed example.
  uint32_t in_pass_counter;
  bool emptylines_separate_examples;  // true if you want to have holdout computed on a per-block basis rather than a
                                      // per-line basis

  std::mutex output_lock;
  std::condition_variable output_done;

  bool done;
  v_array<size_t> gram_mask;

  v_array<size_t> ids;     // unique ids for sources
  v_array<size_t> counts;  // partial examples received from sources
  size_t finished_count;   // the number of finished examples;
  int label_sock;
  int bound_sock;
  int max_fd;

  v_array<substring> parse_name;

  label_parser lp;  // moved from vw

  bool audit;
  bool decision_service_json;
  std::shared_ptr<void> jsonp;
};

parser* new_parser();

void enable_sources(vw& all, bool quiet, size_t passes, input_options& input_options);

bool examples_to_finish();

// only call these from the library form:
void initialize_parser_datastructures(vw& all);

/* [[deprecated]] */
void adjust_used_index(vw& all);

// parser control
void make_example_available();
void lock_done(parser& p);
void set_done(vw& all);

// source control functions
bool inconsistent_cache(size_t numbits, io_buf& cache);
void reset_source(vw& all, size_t numbits);
void finalize_source(parser* source);
void set_compressed(parser* par);
void initialize_examples(vw& all);
void free_parser(vw& all);
