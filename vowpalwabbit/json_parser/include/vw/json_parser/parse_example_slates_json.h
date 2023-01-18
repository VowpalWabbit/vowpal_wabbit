#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/example.h"
#include "vw/core/hashstring.h"
#include "vw/core/label_parser.h"
#include "vw/json_parser/decision_service_utils.h"

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
void parse_slates_example_json(const VW::label_parser& lbl_parser, hash_func_t hash_func, uint64_t hash_seed,
    uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr);

template <bool audit>
void parse_slates_example_json(const VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr);

template <bool audit>
void parse_slates_example_dsjson(VW::workspace& all, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr);

// Define extern template specializations so they don't get initialized when this file is included
extern template void parse_slates_example_json<true>(const VW::label_parser& lbl_parser, hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t length,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
extern template void parse_slates_example_json<false>(const VW::label_parser& lbl_parser, hash_func_t hash_func,
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
}  // namespace details
}  // namespace json
}  // namespace parsers
}  // namespace VW