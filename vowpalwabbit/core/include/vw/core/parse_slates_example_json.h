#pragma once

#include "vw/common/future_compat.h"
#include "vw/json_parser/parse_example_slates_json.h"

template <bool audit>
VW_DEPRECATED("parse_slates_example_json moved to VW::parsers::json::details::parse_slates_example_json")
void parse_slates_example_json(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr)
{
  VW::parsers::json::details::parse_slates_example_json<audit>(lbl_parser, hash_func, hash_seed, parse_mask, chain_hash,
      examples, line, length, std::move(example_factory), dedup_examples);
}

template <bool audit>
VW_DEPRECATED("parse_slates_example_json moved to VW::parsers::json::details::parse_slates_example_json")
void parse_slates_example_json(const VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr)
{
  VW::parsers::json::details::parse_slates_example_json<audit>(
      all, examples, line, length, std::move(example_factory), dedup_examples);
}

template <bool audit>
VW_DEPRECATED("parse_slates_example_dsjson moved to VW::parsers::json::details::parse_slates_example_dsjson")
void parse_slates_example_dsjson(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr)
{
  VW::parsers::json::details::parse_slates_example_dsjson<audit>(
      all, examples, line, length, std::move(example_factory), data, dedup_examples);
}

// Define extern template specializations so they don't get initialized when this file is included
extern template void parse_slates_example_json<true>(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void parse_slates_example_json<false>(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

extern template void parse_slates_example_json<true>(const VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void parse_slates_example_json<false>(const VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

extern template void parse_slates_example_dsjson<true>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void parse_slates_example_dsjson<false>(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
