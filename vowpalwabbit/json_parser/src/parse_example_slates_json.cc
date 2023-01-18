#include "vw/json_parser/parse_example_slates_json.h"

#include "json_utils.h"
#include "vw/common/future_compat.h"

// If the Windows.h header has been included at some point then the GetObject breaks this file.
// Workaround by undefing it for the content of this header.
// We need to put it before the rapidjson header since we cannot assume rapidjson has its
// fix in place or not.
#ifdef GetObject
#  pragma push_macro("GetObject")
#  define VW_WINDOWS_GETOBJECT_MACRO_WAS_UNDEF
#  undef GetObject
#endif

// RapidJson triggers this warning by memcpying non-trivially copyable type. Ignore it so that our warnings are not
// polluted by it.
// https://github.com/Tencent/rapidjson/issues/1700
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_CLASS_MEMACCESS
#include <rapidjson/document.h>
VW_WARNING_STATE_POP

using namespace rapidjson;

namespace
{
inline float get_number(const rapidjson::Value& value)
{
  float number = 0.f;
  if (value.IsInt()) { number = static_cast<float>(value.GetInt()); }
  if (value.IsUint()) { number = static_cast<float>(value.GetUint()); }
  else if (value.IsFloat()) { number = value.GetFloat(); }
  else if (value.IsDouble()) { number = static_cast<float>(value.GetDouble()); }
  else { THROW("Tried to get value as number, but type was " << value.GetType()); }

  return number;
}

template <bool audit>
void handle_features_value(const char* key_namespace, const Value& value, VW::example* current_example,
    std::vector<VW::parsers::json::details::namespace_builder<audit>>& namespaces, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash)
{
  assert(key_namespace != nullptr);
  assert(std::strlen(key_namespace) != 0);
  // Check if it is a reserved field, and if so move on.
  if (key_namespace[0] == '_') { return; }

  const auto key_namespace_length = std::strlen(key_namespace);
  switch (value.GetType())
  {
    case rapidjson::kNullType:
      // Do nothing?
      THROW("Null fields not supported")
      break;
    case rapidjson::kFalseType:
      // No nothing for false!
      assert(true);
      break;
    case rapidjson::kTrueType:
      assert(!namespaces.empty());
      namespaces.back().add_feature(key_namespace, hash_func, parse_mask);
      break;
    case rapidjson::kObjectType:
    {
      push_ns(current_example, key_namespace, namespaces, hash_func, hash_seed);
      for (auto& object_value : value.GetObject())
      {
        handle_features_value(object_value.name.GetString(), object_value.value, current_example, namespaces, hash_func,
            hash_seed, parse_mask, chain_hash);
      }
      pop_ns(current_example, namespaces);
    }
    break;
    case rapidjson::kArrayType:
    {
      push_ns(current_example, key_namespace, namespaces, hash_func, hash_seed);
      auto array_hash = namespaces.back().namespace_hash;

      for (auto& array_value : value.GetArray())
      {
        switch (array_value.GetType())
        {
          case rapidjson::kNumberType:
          {
            float number = get_number(array_value);
            if (audit)
            {
              std::stringstream str;
              str << '[' << (array_hash - namespaces.back().namespace_hash) << ']';
              namespaces.back().add_feature(number, array_hash, str.str().c_str());
            }
            else { namespaces.back().add_feature(number, array_hash, nullptr); }
            array_hash++;
          }
          break;
          case rapidjson::kObjectType:
          {
            handle_features_value(
                key_namespace, array_value, current_example, namespaces, hash_func, hash_seed, parse_mask, chain_hash);
          }
          break;
          default:
            THROW("NOT HANDLED")
        }
      }
      pop_ns(current_example, namespaces);
    }
    break;
    case rapidjson::kStringType:
    {
      assert(!namespaces.empty());
      const char* str = value.GetString();
      // String escape
      const char* end = str + value.GetStringLength();
      for (char* p = const_cast<char*>(str); p != end; p++)
      {
        switch (*p)
        {
          case ' ':
          case '\t':
          case '|':
          case ':':
            *p = '_';
        }
      }

      if (chain_hash) { namespaces.back().add_feature(key_namespace, str, hash_func, parse_mask); }
      else
      {
        char* prepend = const_cast<char*>(str) - key_namespace_length;
        std::memmove(prepend, key_namespace, key_namespace_length);
        namespaces.back().add_feature(prepend, hash_func, parse_mask);
      }
    }

    break;
    case rapidjson::kNumberType:
    {
      assert(!namespaces.empty());
      float number = get_number(value);
      auto hash_index = hash_func(key_namespace, strlen(key_namespace), namespaces.back().namespace_hash) & parse_mask;
      namespaces.back().add_feature(number, hash_index, key_namespace);
    }
    break;
    default:
      break;
  }
}

template <bool audit>
void parse_context(const Value& context, const VW::label_parser& lbl_parser, VW::hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples,
    VW::example_factory_t example_factory, VW::multi_ex& slot_examples,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples = nullptr)
{
  std::vector<VW::parsers::json::details::namespace_builder<audit>> namespaces;
  handle_features_value(" ", context, examples[0], namespaces, hash_func, hash_seed, parse_mask, chain_hash);
  lbl_parser.default_label(examples[0]->l);
  if (context.HasMember("_slot_id"))
  {
    auto slot_id = context["_slot_id"].GetInt();
    examples[0]->l.slates.slot_id = slot_id;
    examples[0]->l.slates.type = VW::slates::example_type::ACTION;
  }
  else { examples[0]->l.slates.type = VW::slates::example_type::SHARED; }

  assert(namespaces.size() == 0);

  if (context.HasMember("_multi"))
  {
    const auto& multi = context["_multi"].GetArray();

    for (const Value& obj : multi)
    {
      auto ex = &example_factory();
      lbl_parser.default_label(ex->l);
      ex->l.slates.type = VW::slates::example_type::ACTION;
      examples.push_back(ex);
      if (dedup_examples && !dedup_examples->empty() && obj.HasMember("__aid"))
      {
        auto dedup_id = obj["__aid"].GetUint64();

        if (dedup_examples->find(dedup_id) == dedup_examples->end()) { THROW("dedup id not found: " << dedup_id); }

        auto* stored_ex = (*dedup_examples)[dedup_id];
        ex->indices = stored_ex->indices;
        for (auto& ns : ex->indices) { ex->feature_space[ns] = stored_ex->feature_space[ns]; }
        ex->ft_offset = stored_ex->ft_offset;
        ex->l.slates.slot_id = stored_ex->l.slates.slot_id;
      }
      else
      {
        auto slot_id = obj["_slot_id"].GetInt();
        ex->l.slates.slot_id = slot_id;
        handle_features_value(" ", obj, ex, namespaces, hash_func, hash_seed, parse_mask, chain_hash);
        assert(namespaces.size() == 0);
      }
    }
  }

  if (context.HasMember("_slots"))
  {
    const auto& slots = context["_slots"].GetArray();
    for (const Value& slot_object : slots)
    {
      auto ex = &example_factory();
      lbl_parser.default_label(ex->l);
      ex->l.slates.type = VW::slates::example_type::SLOT;
      examples.push_back(ex);
      slot_examples.push_back(ex);
      handle_features_value(" ", slot_object, ex, namespaces, hash_func, hash_seed, parse_mask, chain_hash);
      assert(namespaces.size() == 0);
    }
  }
}
}  // namespace

template <bool audit>
void VW::parsers::json::details::parse_slates_example_json(const VW::label_parser& lbl_parser, hash_func_t hash_func,
    uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line, size_t /*length*/,
    VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples)
{
  Document document;
  document.ParseInsitu(line);

  // Build shared example
  const Value& context = document.GetObject();
  VW::multi_ex slot_examples;
  parse_context<audit>(context, lbl_parser, hash_func, hash_seed, parse_mask, chain_hash, examples,
      std::move(example_factory), slot_examples, dedup_examples);
}

template <bool audit>
void VW::parsers::json::details::parse_slates_example_json(const VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples)
{
  parse_slates_example_json<audit>(all.example_parser->lbl_parser, all.example_parser->hasher, all.hash_seed,
      all.parse_mask, all.chain_hash_json, examples, line, length, std::move(example_factory), dedup_examples);
}

template <bool audit>
void VW::parsers::json::details::parse_slates_example_dsjson(VW::workspace& all, VW::multi_ex& examples, char* line,
    size_t /*length*/, VW::example_factory_t example_factory, VW::parsers::json::decision_service_interaction* data,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples)
{
  Document document;
  document.ParseInsitu(line);
  // Build shared example
  const Value& context = document["c"].GetObject();
  VW::multi_ex slot_examples;
  parse_context<audit>(context, all.example_parser->lbl_parser, all.example_parser->hasher, all.hash_seed,
      all.parse_mask, all.chain_hash_json, examples, std::move(example_factory), slot_examples, dedup_examples);

  if (document.HasMember("_label_cost"))
  {
    examples[0]->l.slates.cost = document["_label_cost"].GetFloat();
    for (auto ex : examples) { ex->l.slates.labeled = true; }
  }

  if (document.HasMember("EventId")) { data->event_id = document["EventId"].GetString(); }

  if (document.HasMember("Timestamp")) { data->timestamp = document["Timestamp"].GetString(); }

  if (document.HasMember("_skipLearn")) { data->skip_learn = document["_skipLearn"].GetBool(); }

  if (document.HasMember("pdrop")) { data->probability_of_drop = document["pdrop"].GetFloat(); }

  if (document.HasMember("_outcomes"))
  {
    const auto& outcomes = document["_outcomes"].GetArray();
    assert(outcomes.Size() == slot_examples.size());
    for (rapidjson::SizeType i = 0; i < outcomes.Size(); i++)
    {
      auto& current_obj = outcomes[i];
      auto& destination = slot_examples[i]->l.slates.probabilities;
      auto& actions = current_obj["_a"];

      if (actions.GetType() == rapidjson::kNumberType) { destination.push_back({actions.GetUint(), 0.f}); }
      else if (actions.GetType() == rapidjson::kArrayType)
      {
        for (auto& val : actions.GetArray()) { destination.push_back({val.GetUint(), 0.f}); }
      }
      else { assert(false); }

      auto& probs = current_obj["_p"];
      if (probs.GetType() == rapidjson::kNumberType)
      {
        assert(destination.size() != 0);
        destination[0].score = probs.GetFloat();
      }
      else if (probs.GetType() == rapidjson::kArrayType)
      {
        assert(probs.Size() == destination.size());
        const auto& probs_array = probs.GetArray();
        for (rapidjson::SizeType j = 0; j < probs_array.Size(); j++)
        {
          destination[j].score = probs_array[j].GetFloat();
        }
      }
      else { assert(false); }

      if (current_obj.HasMember("_original_label_cost"))
      {
        assert(current_obj["_original_label_cost"].IsFloat());
        data->original_label_cost = current_obj["_original_label_cost"].GetFloat();
      }
    }

    for (const auto& slot : slot_examples)
    {
      const auto& slates_label = slot->l.slates;
      if (slates_label.labeled)
      {
        data->probabilities.push_back(slates_label.probabilities[0].score);
        data->actions.push_back(slates_label.probabilities[0].action);
      }
    }
  }
}

// Explicitly instantiate templates only in this source file
template void VW::parsers::json::details::parse_slates_example_json<true>(const VW::label_parser& lbl_parser,
    hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::parsers::json::details::parse_slates_example_json<false>(const VW::label_parser& lbl_parser,
    hash_func_t hash_func, uint64_t hash_seed, uint64_t parse_mask, bool chain_hash, VW::multi_ex& examples, char* line,
    size_t length, VW::example_factory_t example_factory, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void VW::parsers::json::details::parse_slates_example_json<true>(const VW::workspace& all,
    VW::multi_ex& examples, char* line, size_t length, VW::example_factory_t example_factory,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::parsers::json::details::parse_slates_example_json<false>(const VW::workspace& all,
    VW::multi_ex& examples, char* line, size_t length, VW::example_factory_t example_factory,
    std::unordered_map<uint64_t, VW::example*>* dedup_examples);

template void VW::parsers::json::details::parse_slates_example_dsjson<true>(VW::workspace& all, VW::multi_ex& examples,
    char* line, size_t length, VW::example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data, std::unordered_map<uint64_t, VW::example*>* dedup_examples);
template void VW::parsers::json::details::parse_slates_example_dsjson<false>(VW::workspace& all, VW::multi_ex& examples,
    char* line, size_t length, VW::example_factory_t example_factory,
    VW::parsers::json::decision_service_interaction* data, std::unordered_map<uint64_t, VW::example*>* dedup_examples);

#ifdef VW_WINDOWS_GETOBJECT_MACRO_WAS_UNDEF
#  pragma pop_macro("GetObject")
#  undef VW_WINDOWS_GETOBJECT_MACRO_WAS_UNDEF
#endif
