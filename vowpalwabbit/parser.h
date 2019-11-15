/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "io_buf.h"
#include "parse_primitives.h"
#include "example.h"
#include "future_compat.h"

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

struct example_initializer
{
  example* operator()(example* ex);
};

struct parser
{
  parser(size_t ring_size, bool strict_parse_)
      : example_pool{ring_size}, ready_parsed_examples{ring_size}, ring_size{ring_size}, strict_parse{strict_parse_}
  {
    this->input = new io_buf{};
    this->output = new io_buf{};
    this->lp = simple_label;

    // Free parser must still be used for the following fields.
    this->words = v_init<substring>();
    this->name = v_init<substring>();
    this->parse_name = v_init<substring>();
    this->gram_mask = v_init<size_t>();
    this->ids = v_init<size_t>();
    this->counts = v_init<size_t>();
  }

  ~parser()
  {
    delete input;
    delete output;
  }

  // helper(s) for text parsing
  v_array<substring> words;
  v_array<substring> name;

  VW::object_pool<example, example_initializer> example_pool;
  VW::ptr_queue<example> ready_parsed_examples;

  io_buf* input = nullptr;  // Input source(s)
  int (*reader)(vw*, v_array<example*>& examples);
  void (*text_reader)(vw*, char*, size_t, v_array<example*>&);

  hash_func_t hasher;
  bool resettable;           // Whether or not the input can be reset.
  io_buf* output = nullptr;  // Where to output the cache.
  bool write_cache = false;
  bool sort_features = false;
  bool sorted_cache = false;

  const size_t ring_size;
  uint64_t begin_parsed_examples = 0;  // The index of the beginning parsed example.
  uint64_t end_parsed_examples = 0;    // The index of the fully parsed example.
  uint32_t in_pass_counter = 0;
  bool emptylines_separate_examples = false;  // true if you want to have holdout computed on a per-block basis rather
                                              // than a per-line basis

  std::mutex output_lock;
  std::condition_variable output_done;

  bool done = false;
  v_array<size_t> gram_mask;

  v_array<size_t> ids;     // unique ids for sources
  v_array<size_t> counts;  // partial examples received from sources
  size_t finished_count;   // the number of finished examples;
  int label_sock = 0;
  int bound_sock = 0;
  int max_fd = 0;

  v_array<substring> parse_name;

  label_parser lp;  // moved from vw

  bool audit = false;
  bool decision_service_json = false;

  bool strict_parse;
  std::exception_ptr exc_ptr;
};

void enable_sources(vw& all, bool quiet, size_t passes, input_options& input_options);

VW_DEPRECATED("Function is no longer used")
void adjust_used_index(vw& all);

// parser control
void lock_done(parser& p);
void set_done(vw& all);

// source control functions
void reset_source(vw& all, size_t numbits);
void finalize_source(parser* source);
void set_compressed(parser* par);
void free_parser(vw& all);
