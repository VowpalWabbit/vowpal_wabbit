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

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/common/future_compat.h"
#include "vw/common/string_view.h"
#include "vw/core/example.h"
#include "vw/core/hashstring.h"
#include "vw/core/io_buf.h"
#include "vw/core/object_pool.h"
#include "vw/core/queue.h"
#include "vw/core/vw_fwd.h"

#include <atomic>
#include <memory>

namespace VW
{
namespace details
{
class dsjson_metrics;
}

void parse_example_label(string_view label, const VW::label_parser& lbl_parser, const named_labels* ldict,
    label_parser_reuse_mem& reuse_mem, example& ec, VW::io::logger& logger);
void setup_examples(VW::workspace& all, VW::multi_ex& examples);

VW::example& get_unused_example(VW::workspace* all);

class parser
{
public:
  parser(size_t example_queue_limit, bool strict_parse_);

  // delete copy constructor
  parser(const parser&) = delete;
  parser& operator=(const parser&) = delete;

  // helper(s) for text parsing
  std::vector<VW::string_view> words;

  VW::object_pool<VW::example> example_pool;
  VW::thread_safe_queue<VW::example*> ready_parsed_examples;

  io_buf input;  // Input source(s)

  /// reader consumes the given io_buf and produces parsed examples. The number
  /// of produced examples is implementation defined. However, in practice for
  /// single_line parsers a single example is produced. And for multi_line
  /// parsers multiple are produced which all correspond the the same overall
  /// logical example. examples must have a single empty example in it when this
  /// call is made.
  int (*reader)(VW::workspace*, io_buf&, VW::multi_ex& examples);
  /// text_reader consumes the char* input and is for text based parsing
  void (*text_reader)(VW::workspace*, VW::string_view, VW::multi_ex&);

  hash_func_t hasher;
  bool resettable;  // Whether or not the input can be reset.
  io_buf output;    // Where to output the cache.
  VW::parsers::cache::details::cache_temp_buffer cache_temp_buffer_obj;
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
  std::unique_ptr<details::dsjson_metrics> metrics = nullptr;
};
namespace details
{
class input_options;

class dsjson_metrics
{
public:
  size_t number_of_skipped_events = 0;
  size_t number_of_events_zero_actions = 0;
  size_t line_parse_error = 0;
  float dsjson_sum_cost_original = 0.f;
  float dsjson_sum_cost_original_first_slot = 0.f;
  float dsjson_sum_cost_original_baseline = 0.f;
  size_t dsjson_number_of_label_equal_baseline_first_slot = 0;
  size_t dsjson_number_of_label_not_equal_baseline_first_slot = 0;
  float dsjson_sum_cost_original_label_equal_baseline_first_slot = 0.f;
  std::string first_event_id;
  std::string first_event_time;
  std::string last_event_id;
  std::string last_event_time;
};

void enable_sources(VW::workspace& all, bool quiet, size_t passes, const VW::details::input_options& input_options);

// parser control
void lock_done(parser& p);
void set_done(VW::workspace& all);

// source control functions
void reset_source(VW::workspace& all, size_t numbits);
void free_parser(VW::workspace& all);
}  // namespace details
}  // namespace VW

using parser VW_DEPRECATED("Use VW::parser instead of ::parser. ::parser will be removed in VW 10.") = VW::parser;
