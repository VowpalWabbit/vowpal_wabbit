// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/label_parser.h"
#include "vw/core/parse_example.h"
#include "vw/core/parser.h"
#include "vw/core/v_array.h"
#include "vw/io/logger.h"
#include "vw/json_parser/decision_service_utils.h"
#include "vw/json_parser/parse_example_slates_json.h"

#include <set>
#include <string>
#include <unordered_map>

namespace VW
{
namespace parsers
{
namespace json
{
namespace details
{
template <bool audit>
bool parse_line_json(VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);
}

template <bool audit>
void read_line_json(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask,
    bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict, VW::multi_ex& examples,
    char* line, size_t length, example_factory_t example_factory, VW::io::logger& logger,
    std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr);

template <bool audit>
void read_line_json(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr);

// returns true if succesfully parsed, returns false if not and logs warning
template <bool audit>
bool read_line_decision_service_json(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    bool copy_line, example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data);

// This is used by the python parser
template <bool audit>
void line_to_examples_json(VW::workspace* all, VW::string_view sv, VW::multi_ex& examples);

template <bool audit>
int read_features_json(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);

// Define extern template specializations so they don't get initialized when this file is included
extern template void read_line_json<true>(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict,
    VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory, VW::io::logger& logger,
    std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void read_line_json<false>(const VW::label_parser& lbl_parser, hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem,
    const VW::named_labels* ldict, VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory,
    VW::io::logger& logger, std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);

extern template void read_line_json<true>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void read_line_json<false>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

extern template bool read_line_decision_service_json<true>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);
extern template bool read_line_decision_service_json<false>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);

namespace details
{
extern template bool parse_line_json<true>(VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);
extern template bool parse_line_json<false>(VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);
}  // namespace details

extern template void line_to_examples_json<true>(VW::workspace* all, VW::string_view, VW::multi_ex& examples);
extern template void line_to_examples_json<false>(VW::workspace* all, VW::string_view, VW::multi_ex& examples);

extern template int read_features_json<true>(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);
extern template int read_features_json<false>(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);

}  // namespace json
}  // namespace parsers
}  // namespace VW