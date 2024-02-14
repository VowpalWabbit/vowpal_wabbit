// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/api_status.h"
#include "vw/core/example.h"
#include "vw/core/multi_ex.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw_fwd.h"
#include "vw/fb_parser/generated/example_generated.h"

namespace VW
{

class api_status;

using example_sink_f = std::function<void(VW::multi_ex&& spare_examples)>;

namespace parsers
{
namespace flatbuffer
{
int flatbuffer_to_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples);


bool read_span_flatbuffer(
    VW::workspace* all, const uint8_t* span, size_t length, example_factory_t example_factory, VW::multi_ex& examples, example_sink_f example_sink = nullptr);

class parser
{
public:
  parser() = default;
  const VW::parsers::flatbuffer::ExampleRoot* data();
  int parse_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples, const uint8_t* buffer_pointer = nullptr,
      VW::experimental::api_status* status = nullptr);

private:
  size_t _num_example_roots = 0;
  const VW::parsers::flatbuffer::ExampleRoot* _data;
  const uint8_t* _flatbuffer_pointer;
  flatbuffers::uoffset_t _object_size = 0;
  bool _active_collection = false;
  uint32_t _example_index = 0;
  uint32_t _multi_ex_index = 0;
  bool _active_multi_ex = false;
  const VW::parsers::flatbuffer::MultiExample* _multi_example_object = nullptr;
  uint32_t _labeled_action = 0;
  uint64_t _c_hash = 0;

  int parse(io_buf& buf, const uint8_t* buffer_pointer = nullptr, VW::experimental::api_status* status = nullptr);
  int process_collection_item(
      VW::workspace* all, VW::multi_ex& examples, VW::experimental::api_status* status = nullptr);
  int parse_example(VW::workspace* all, example* ae, const Example* eg, VW::experimental::api_status* status = nullptr);
  int parse_multi_example(
      VW::workspace* all, example* ae, const MultiExample* eg, VW::experimental::api_status* status = nullptr);
  int parse_namespaces(
      VW::workspace* all, example* ae, const Namespace* ns, VW::experimental::api_status* status = nullptr);
  int parse_flat_label(shared_data* sd, example* ae, const Example* eg, VW::io::logger& logger,
      VW::experimental::api_status* status = nullptr);
  int get_namespace_index(const Namespace* ns, namespace_index& ni, VW::experimental::api_status* status = nullptr);

  void parse_simple_label(shared_data* sd, polylabel* l, reduction_features* red_features, const SimpleLabel* label);
  void parse_cb_label(polylabel* l, const CBLabel* label);
  void parse_ccb_label(polylabel* l, const CCBLabel* label);
  void parse_cs_label(polylabel* l, const CS_Label* label);
  void parse_cb_eval_label(polylabel* l, const CB_EVAL_Label* label);
  void parse_mc_label(shared_data* sd, polylabel* l, const MultiClass* label, VW::io::logger& logger);
  void parse_multi_label(polylabel* l, const MultiLabel* label);
  int parse_slates_label(polylabel* l, const Slates_Label* label, VW::experimental::api_status* status = nullptr);
  void parse_continuous_action_label(polylabel* l, const ContinuousLabel* label);
};
}  // namespace flatbuffer
}  // namespace parsers
}  // namespace VW
