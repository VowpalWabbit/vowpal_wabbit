// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/parse_example_json.h"

// Explicitly instantiate templates only in this source file
template void VW::read_line_json_s<true>(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict,
    VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory, VW::io::logger& logger,
    std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::read_line_json_s<false>(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::label_parser_reuse_mem* reuse_mem, const VW::named_labels* ldict,
    VW::multi_ex& examples, char* line, size_t length, example_factory_t example_factory, VW::io::logger& logger,
    std::unordered_map<std::string, std::set<std::string>>* ignore_features,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void VW::read_line_json_s<true>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::read_line_json_s<false>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template bool VW::read_line_decision_service_json<true>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);
template bool VW::read_line_decision_service_json<false>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, bool copy_line, example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data);

template bool parse_line_json<true>(VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);
template bool parse_line_json<false>(VW::workspace* all, char* line, size_t num_chars, VW::multi_ex& examples);

template void line_to_examples_json<true>(VW::workspace* all, VW::string_view sv, VW::multi_ex& examples);
template void line_to_examples_json<false>(VW::workspace* all, VW::string_view sv, VW::multi_ex& examples);

template int read_features_json<true>(VW::workspace* all, VW::io_buf& buf, VW::multi_ex& examples);
template int read_features_json<false>(VW::workspace* all, VW::io_buf& buf, VW::multi_ex& examples);
