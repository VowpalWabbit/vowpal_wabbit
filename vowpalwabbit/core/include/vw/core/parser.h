// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

// Mutex and CV cannot be used in managed C++, tell the compiler that this is unmanaged even if included in a managed
// project.
#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <condition_variable>
#  include <mutex>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <condition_variable>
#  include <mutex>
#endif

#include "hashstring.h"
#include "object_pool.h"
#include "queue.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/example.h"
#include "vw/core/io_buf.h"
#include "vw/core/vw_fwd.h"

#include <atomic>
#include <memory>

namespace VW
{
void parse_example_label(string_view label, const VW::label_parser& lbl_parser, const named_labels* ldict,
    label_parser_reuse_mem& reuse_mem, example& ec, VW::io::logger& logger);
void setup_examples(VW::workspace& all, VW::multi_ex& examples);
namespace details
{
struct cache_temp_buffer
{
  std::shared_ptr<std::vector<char>> _backing_buffer;
  io_buf _temporary_cache_buffer;
  cache_temp_buffer()
  {
    _backing_buffer = std::make_shared<std::vector<char>>();
    _temporary_cache_buffer.add_file(VW::io::create_vector_writer(_backing_buffer));
  }
};
}  // namespace details
}  // namespace VW

struct input_options;
struct dsjson_metrics;

struct parser
{
  parser(size_t example_queue_limit, bool strict_parse_);

  // delete copy constructor
  parser(const parser&) = delete;
  parser& operator=(const parser&) = delete;

  // helper(s) for text parsing
  std::vector<VW::string_view> words;

  VW::object_pool<VW::example> example_pool;
  VW::ptr_queue<VW::example> ready_parsed_examples;

  io_buf input;  // Input source(s)

  /// reader consumes the given io_buf and produces parsed examples. The number
  /// of produced examples is implementation defined. However, in practice for
  /// single_line parsers a single example is produced. And for multi_line
  /// parsers multiple are produced which all correspond the the same overall
  /// logical example. examples must have a single empty example in it when this
  /// call is made.
  int (*reader)(VW::workspace*, io_buf&, VW::multi_ex& examples);
  /// text_reader consumes the char* input and is for text based parsing
  void (*text_reader)(VW::workspace*, const char*, size_t, VW::multi_ex&);

  shared_data* _shared_data = nullptr;

  hash_func_t hasher;
  bool resettable;  // Whether or not the input can be reset.
  io_buf output;    // Where to output the cache.
  VW::details::cache_temp_buffer _cache_temp_buffer;
  std::string currentname;
  std::string finalname;

  bool write_cache = false;
  bool sort_features = false;

  size_t example_queue_limit;
  std::atomic<uint64_t> num_examples_taken_from_pool;
  std::atomic<uint64_t> num_setup_examples;
  std::atomic<uint64_t> num_finished_examples;
  uint32_t in_pass_counter = 0;
  bool emptylines_separate_examples = false;  // true if you want to have holdout computed on a per-block basis rather
                                              // than a per-line basis

  std::mutex output_lock;
  std::condition_variable output_done;

  bool done = false;

  int bound_sock = 0;

  VW::label_parser_reuse_mem parser_memory_to_reuse;

  VW::label_parser lbl_parser;  // moved from vw

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
  float DsjsonSumCostOriginalBaseline = 0.f;
  size_t DsjsonNumberOfLabelEqualBaselineFirstSlot = 0;
  size_t DsjsonNumberOfLabelNotEqualBaselineFirstSlot = 0;
  float DsjsonSumCostOriginalLabelEqualBaselineFirstSlot = 0.f;
  std::string FirstEventId;
  std::string FirstEventTime;
  std::string LastEventId;
  std::string LastEventTime;
};

void enable_sources(VW::workspace& all, bool quiet, size_t passes, input_options& input_options);

// parser control
void lock_done(parser& p);
void set_done(VW::workspace& all);

// source control functions
void reset_source(VW::workspace& all, size_t numbits);
void free_parser(VW::workspace& all);
