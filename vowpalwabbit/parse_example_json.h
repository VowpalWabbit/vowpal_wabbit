// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "example.h"
#include "hashstring.h"
#include "io_buf.h"
#include "label_parser.h"
#include "label_type.h"
#include "memory.h"
#include "named_labels.h"
#include "parser.h"
#include "v_array.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

struct DecisionServiceInteraction;

namespace VW
{
struct workspace;
}

namespace VW
{
namespace details
{
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
void persist_dsjson_metrics(VW::metric_sink& sink, VW::label_type_t label_type, const details::dsjson_metrics& metrics);
}  // namespace details

struct json_example_parser : VW::example_parser_i
{
  json_example_parser(VW::label_type_t label_type, hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask,
      bool chain_hash, std::unique_ptr<VW::example_factory_i> example_factory, const named_labels* ldict, bool audit);

  bool next_with_dedup(
      io_buf& input, const std::unordered_map<uint64_t, example*>* dedup_examples, v_array<example*>& output);

  bool next(io_buf& input, v_array<example*>& output) override;

  // This function is specifically here if you don't want to drain from an iobuf. In the context of json this is useful as in tests it is convenient to not have to flatten to a single line.
  // Since the format is officially newline delimited JSON the standard way of parsing is consuming a single line. Whereas this will parse the entire given buffer.
  // Must be only a single JSON object in the given bytes. 
  void parse_object(char* line, size_t length, const std::unordered_map<uint64_t, example*>* dedup_examples,
      v_array<example*>& output);

private:
  template <bool audit>
  void parse_line(char* line, size_t length,
      const std::unordered_map<uint64_t, example*>* dedup_examples, v_array<example*>& examples);

  label_parser _label_parser;
  hash_func_t _hash_func;
  uint64_t _hash_seed;
  uint64_t _parse_mask;
  bool _chain_hash;
  std::unique_ptr<VW::example_factory_i> _example_factory;
  const named_labels* _ldict;
  bool _audit;

  VW::label_parser_reuse_mem _parser_mem;
};

struct dsjson_example_parser : VW::example_parser_i
{
  dsjson_example_parser(VW::label_type_t label_type, hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask,
      bool chain_hash, std::unique_ptr<VW::example_factory_i> example_factory, const named_labels* ldict, bool audit,
      bool record_metrics, bool destructive_parse, bool strict_parse);

  bool next(io_buf& input, v_array<example*>& output) override;
  bool next_with_interaction(io_buf& input, v_array<example*>& output, DecisionServiceInteraction& interaction);
  void persist_metrics(VW::metric_sink& sink) override;

  // This function is specifically here if you don't want to drain from an iobuf. In the context of json this is useful as in tests it is convenient to not have to flatten to a single line.
  // Since the format is officially newline delimited JSON the standard way of parsing is consuming a single line. Whereas this will parse the entire given buffer.
  // Must be only a single JSON object in the given bytes. 
   void parse_object(char* line, size_t length, 
      v_array<example*>& output, DecisionServiceInteraction& interaction);

private:
  template <bool audit>
  bool parse_line_and_process_metrics(char* line, size_t num_chars, VW::details::dsjson_metrics* metrics,
      v_array<example*>& examples, DecisionServiceInteraction& interaction);

  template <bool audit>
  bool parse_line(char* line, size_t length, v_array<example*>& examples, DecisionServiceInteraction* data);

  label_parser _label_parser;
  hash_func_t _hash_func;
  uint64_t _hash_seed;
  uint64_t _parse_mask;
  bool _chain_hash;
  std::unique_ptr<VW::example_factory_i> _example_factory;
  const named_labels* _ldict;
  bool _audit;
  bool _should_record_metrics;
  bool _destructive_parse;
  bool _strict_parse;

  details::dsjson_metrics _ds_metrics;
  VW::label_parser_reuse_mem _parser_mem;
};

std::unique_ptr<VW::json_example_parser> make_json_parser(VW::workspace& all);
std::unique_ptr<VW::dsjson_example_parser> make_dsjson_parser(
    VW::workspace& all, bool record_metrics, bool destructive_parse, bool strict_parse);
}  // namespace VW
