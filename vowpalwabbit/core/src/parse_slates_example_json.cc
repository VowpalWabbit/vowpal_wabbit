#include "vw/core/parse_slates_example_json.h"

#include "vw/common/future_compat.h"

// Explicitly instantiate templates only in this source file
template void parse_slates_example_json<true>(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void parse_slates_example_json<false>(const VW::label_parser& lbl_parser, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void parse_slates_example_json<true>(const VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void parse_slates_example_json<false>(const VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void parse_slates_example_dsjson<true>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void parse_slates_example_dsjson<false>(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);