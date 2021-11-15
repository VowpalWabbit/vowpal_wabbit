// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once
#include "io_buf.h"
#include "example.h"
#include "future_compat.h"
#include "named_labels.h"

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
#include "metric_sink.h"

namespace VW
{
struct workspace;

void parse_example_label(VW::string_view label, const label_parser& lbl_parser, const named_labels* ldict,
    label_parser_reuse_mem& reuse_mem, example& ec);

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

struct example_factory_i
{
  virtual ~example_factory_i() = default;
  virtual example* create() = 0;
  virtual void destroy(example*) = 0;
};

std::unique_ptr<example_factory_i> make_example_factory(VW::workspace& ws);

struct example_parser_i
{
  example_parser_i(std::string type);
  virtual ~example_parser_i() = default;

  // False if there is no next - EOF
  // If io_buf is changed without first calling reset() it is UB
  virtual bool next(io_buf& input, v_array<example*>& output) = 0;

  // ! - the io_buf will change
  // Some example parsers pre-parse the io_buf so an explicit signal is required to prepare them for a new io_buf
  virtual void reset() {}

  virtual void persist_metrics(metric_sink& /*sink*/) {}

  VW::string_view type();
private:
  std::string _type;
};

struct example_parser_factory_i
{
  virtual ~example_parser_factory_i() = default;

  virtual std::unique_ptr<example_parser_i> make_parser(bool audit,
      VW::label_type_t type, uint32_t hash_seed, uint64_t parse_mask, hash_func_t hash_func, bool record_metrics, const named_labels* ldict, bool chain_hash,
      std::unique_ptr<example_factory_i>&& example_factory) = 0;
};

}  // namespace VW

struct input_options;
struct dsjson_metrics;
struct parser
{
  parser(size_t ring_size, bool strict_parse_)
      : example_pool{ring_size}
      , ready_parsed_examples{ring_size}
      , ring_size{ring_size}
      , num_examples_taken_from_pool(0)
      , num_setup_examples(0)
      , num_finished_examples(0)
      , strict_parse{strict_parse_}
  {
    this->lbl_parser = simple_label_parser;
  }

  // delete copy constructor
  parser(const parser&) = delete;
  parser& operator=(const parser&) = delete;

  VW::object_pool<example> example_pool;
  VW::ptr_queue<example> ready_parsed_examples;

  io_buf input;  // Input source(s)

  /// reader consumes the given io_buf and produces parsed examples. The number
  /// of produced examples is implementation defined. However, in practice for
  /// single_line parsers a single example is produced. And for multi_line
  /// parsers multiple are produced which all correspond the the same overall
  /// logical example. examples must have a single empty example in it when this
  /// call is made.
  std::unique_ptr<VW::example_parser_i> active_example_parser = nullptr;

  std::unique_ptr<VW::example_parser_factory_i> custom_example_parser_factory = nullptr;


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


  label_parser lbl_parser;  // moved from vw

  bool strict_parse;
  std::exception_ptr exc_ptr;

  // Helper state to reduce allocations.
  VW::label_parser_reuse_mem parser_memory_to_reuse;
  VW::details::cache_temp_buffer _cache_temp_buffer;
};

void enable_sources(VW::workspace& all, bool quiet, size_t passes, input_options& input_options);

// parser control
void lock_done(parser& p);
void set_done(VW::workspace& all);

// source control functions
void reset_source(VW::workspace& all, size_t numbits);
void free_parser(VW::workspace& all);
