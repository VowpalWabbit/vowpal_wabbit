// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "../../vw.h"
#include "../../parser.h"
VW_WARNING_STATE_PUSH
VW_WARNING_DISABLE_BADLY_FORMED_XML
#include "generated/example_generated.h"
VW_WARNING_STATE_POP

namespace VW
{
namespace parsers
{
namespace flatbuffer
{
class parser : public VW::example_parser_i
{
public:
  parser(VW::label_type_t label_type, bool audit, const VW::named_labels* ldict, hash_func_t hash_function,
      uint64_t hash_seed)
      : example_parser_i("flatbuffer")
      , _label_parser(VW::get_label_parser(label_type))
      , _audit(audit)
      , _ldict(ldict)
      , _hash_function(hash_function)
      , _hash_seed(hash_seed)
  {
  }
  const VW::parsers::flatbuffer::ExampleRoot* data();
  bool parse_examples(io_buf& buf, v_array<example*>& examples, uint8_t* buffer_pointer = nullptr);

  bool next(io_buf& input, v_array<example*>& output) override;
  void reset() override;

private:
  label_parser _label_parser;
  bool _audit;
  const VW::named_labels* _ldict;
  hash_func_t _hash_function;
  uint64_t _hash_seed;

  const VW::parsers::flatbuffer::ExampleRoot* _data = nullptr;
  uint8_t* _flatbuffer_pointer = nullptr;
  flatbuffers::uoffset_t _object_size = 0;
  bool _active_collection = false;
  uint32_t _example_index = 0;
  uint32_t _multi_ex_index = 0;
  bool _active_multi_ex = false;
  const VW::parsers::flatbuffer::MultiExample* _multi_example_object = nullptr;
  uint32_t _labeled_action = 0;
  uint64_t _c_hash = 0;

  bool parse(io_buf& buf, uint8_t* buffer_pointer = nullptr);
  void process_collection_item(v_array<example*>& examples);
  void parse_example(example* ae, const Example* eg);
  void parse_multi_example(example* ae, const MultiExample* eg);
  void parse_namespaces(example* ae, const Namespace* ns);
  void parse_features(features& fs, const Feature* feature, const flatbuffers::String* ns);
  void parse_flat_label(example* ae, const Example* eg);

  void parse_simple_label(polylabel* l, reduction_features* red_features, const SimpleLabel* label);
  void parse_cb_label(polylabel* l, const CBLabel* label);
  void parse_ccb_label(polylabel* l, const CCBLabel* label);
  void parse_cs_label(polylabel* l, const CS_Label* label);
  void parse_cb_eval_label(polylabel* l, const CB_EVAL_Label* label);
  void parse_mc_label(const VW::named_labels* ldict, polylabel* l, const MultiClass* label);
  void parse_multi_label(polylabel* l, const MultiLabel* label);
  void parse_slates_label(polylabel* l, const Slates_Label* label);
  void parse_continuous_action_label(polylabel* l, const ContinuousLabel* label);
};

inline std::unique_ptr<parser> make_flatbuffer_parser(vw& all)
{
  return VW::make_unique<parser>(all.example_parser->lbl_parser.label_type, all.audit || all.hash_inv,
      all.sd->ldict.get(), all.example_parser->hasher, all.hash_seed);
}
}  // namespace flatbuffer
}  // namespace parsers
}  // namespace VW
