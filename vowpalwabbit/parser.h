// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "io_buf.h"
#include "example.h"
#include "future_compat.h"

// Mutex and CV cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed
// project.
#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <mutex>
#  include <condition_variable>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <mutex>
#  include <condition_variable>
#endif

#include <atomic>
#include <memory>
#include "vw_string_view.h"
#include "queue.h"
#include "object_pool.h"
#include "hashstring.h"
#include "simple_label_parser.h"

struct vw;
struct input_options;
struct dsjson_metrics;
struct parser
{
  parser(size_t ring_size, bool strict_parse_)
      : example_pool{ring_size}
      , ready_parsed_examples{ring_size}
      , ring_size{ring_size}
      , begin_parsed_examples(0)
      , end_parsed_examples(0)
      , finished_examples(0)
      , strict_parse{strict_parse_}
  {
    this->lbl_parser = simple_label_parser;
  }

  // delete copy constructor
  parser(const parser&) = delete;
  parser& operator=(const parser&) = delete;

  // helper(s) for text parsing
  std::vector<VW::string_view> words;

  VW::object_pool<example> example_pool;
  VW::ptr_queue<example> ready_parsed_examples;

  io_buf input;  // Input source(s)

  /// reader consumes the given io_buf and produces parsed examples. The number
  /// of produced examples is implementation defined. However, in practice for
  /// single_line parsers a single example is produced. And for multi_line
  /// parsers multiple are produced which all correspond the the same overall
  /// logical example. examples must have a single empty example in it when this
  /// call is made.
  int (*reader)(vw*, io_buf&, v_array<example*>& examples);
  /// text_reader consumes the char* input and is for text based parsing
  void (*text_reader)(vw*, const char*, size_t, v_array<example*>&);

  shared_data* _shared_data = nullptr;

  hash_func_t hasher;
  bool resettable;           // Whether or not the input can be reset.
  io_buf output;             // Where to output the cache.
  std::string currentname;
  std::string finalname;

  bool write_cache = false;
  bool sort_features = false;
  bool sorted_cache = false;

  const size_t ring_size;
  std::atomic<uint64_t> begin_parsed_examples;  // The index of the beginning parsed example.
  std::atomic<uint64_t> end_parsed_examples;    // The index of the fully parsed example.
  std::atomic<uint64_t> finished_examples;      // The count of finished examples.
  uint32_t in_pass_counter = 0;
  bool emptylines_separate_examples = false;  // true if you want to have holdout computed on a per-block basis rather
                                              // than a per-line basis

  std::mutex output_lock;
  std::condition_variable output_done;

  bool done = false;

  v_array<size_t> ids;     // unique ids for sources
  v_array<size_t> counts;  // partial examples received from sources
  size_t finished_count;   // the number of finished examples;
  int bound_sock = 0;

  std::vector<VW::string_view> parse_name;

  label_parser lbl_parser;  // moved from vw

  bool audit = false;
  bool decision_service_json = false;

  bool strict_parse;
  std::exception_ptr exc_ptr;
  std::unique_ptr<dsjson_metrics> metrics = nullptr;
};

struct dsjson_metrics
{
  size_t NumberOfSkippedEvents = 0;
  size_t NumberOfEventsZeroActions = 0;
  size_t LineParseError = 0;
  float DsjsonSumCostOriginal = 0.f;
  float DsjsonSumCostOriginalFirstSlot = 0.f;
  std::string FirstEventId;
  std::string FirstEventTime;
  std::string LastEventId;
  std::string LastEventTime;
};

void enable_sources(vw& all, bool quiet, size_t passes, input_options& input_options);

// parser control
void lock_done(parser& p);
void set_done(vw& all);

// source control functions
void reset_source(vw& all, size_t numbits);
void free_parser(vw& all);
