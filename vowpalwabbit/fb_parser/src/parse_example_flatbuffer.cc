// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/fb_parser/parse_example_flatbuffer.h"

#include "vw/core/api_status.h"
#include "vw/core/action_score.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/error_constants.h"
#include "vw/core/global_data.h"
#include "vw/core/parser.h"
#include "vw/core/scope_exit.h"
#include "vw/core/vw.h"

#include <cfloat>
#include <fstream>
#include <iostream>
#include <sstream>

namespace VW
{
namespace parsers
{
namespace flatbuffer
{
int flatbuffer_to_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  VW::experimental::api_status status;
  int result = all->parser_runtime.flat_converter->parse_examples(all, buf, examples, nullptr, &status);
  switch (result)
  {
    case VW::experimental::error_code::success:
      return 1;
    case VW::experimental::error_code::nothing_to_parse:
      return 0;  // this is not a true error, but indicates that the parser is done
    default:
      std::stringstream sstream;
      sstream << "Error parsing examples: " << status.get_error_msg() << std::endl;
      THROW(sstream.str());
  }

  return static_cast<int>(status.get_error_code() == VW::experimental::error_code::success);
}

bool read_span_flatbuffer(VW::workspace* all, const uint8_t* span, size_t length, example_factory_t example_factory,
    VW::multi_ex& examples, example_sink_f example_sink)
{
  int a = 0;
  a++;

  // we expect context to contain a size_prefixed flatbuffer (technically a binary string)
  // which means:
  //
  //    /   4b   /    *length b                           /
  //    +--------+----------------------------------------+
  //    | length | flatbuffer                             |
  //    +--------+----------------------------------------+
  //    |                   context                       |
  //    +--------+----------------------------------------+
  //
  //    thus context.size() = sizeof(length) + length
  io_buf unused;

  // TODO: How do we report errors out of here? (This is a general API problem with the parsers)
  size_t address = reinterpret_cast<size_t>(span);
  if (address % 8 != 0)
  {
    std::stringstream sstream;
    sstream << "fb_parser error: flatbuffer data not aligned to 8 bytes" << std::endl;
    sstream << "   span => @" << std::hex << address << std::dec << " % " << 8 << " = " << address % 8
            << " (vs desired = " << 0 << ")";
    THROW(sstream.str());
    return false;
  }

  flatbuffers::uoffset_t flatbuffer_object_size =
      flatbuffers::ReadScalar<flatbuffers::uoffset_t>(span);  //*reinterpret_cast<const uint32_t*>(span);
  if (length != flatbuffer_object_size + sizeof(flatbuffers::uoffset_t))
  {
    std::stringstream sstream;
    sstream << "fb_parser error: flatbuffer size prefix does not match actual size" << std::endl;
    sstream << "   span => @" << std::hex << address << std::dec << " size_prefix = " << flatbuffer_object_size
            << " length = " << length;
    THROW(sstream.str());
    return false;
  }

  VW::multi_ex temp_ex;
  auto scope_guard = VW::scope_exit([&temp_ex, &all, &example_sink]()
  {
    if (example_sink == nullptr) { VW::finish_example(*all, temp_ex); }
    else { example_sink(std::move(temp_ex)); }
  });

  // There is a bit of unhappiness with the interface of the read_XYZ_<format>() functions, because they often
  // expect the input multi_ex to have a single "empty" example there. This contributes, in part, to the large
  // proliferation of entry points into the JSON parser(s). We want to avoid exposing that insofar as possible,
  // so we will check whether we already received a perfectly good example and use that, or create a new one if
  // needed.
  if (examples.size() > 0)
  {
    temp_ex.push_back(examples[0]);
    examples.pop_back();
  }
  else { temp_ex.push_back(&example_factory()); }

  bool has_more = true;
  VW::experimental::api_status status;

  do {
    switch (all->parser_runtime.flat_converter->parse_examples(all, unused, temp_ex, span, &status))
    {
      case VW::experimental::error_code::success:
        has_more = true;
        break;
      case VW::experimental::error_code::nothing_to_parse:
        has_more = false;
        break;
      default:
        std::stringstream sstream;
        sstream << "Error parsing examples: " << std::endl;
        THROW(sstream.str());
        return false;
    }

    has_more &= !temp_ex[0]->is_newline;

    if (!temp_ex[0]->is_newline)
    {
      examples.push_back(&example_factory());
      std::swap(examples[examples.size() - 1], temp_ex[0]);
    }
  } while (has_more);

  return true;
}

const VW::parsers::flatbuffer::ExampleRoot* parser::data() { return _data; }

int parser::parse(io_buf& buf, const uint8_t* buffer_pointer, VW::experimental::api_status* status)
{
#define RETURN_IF_ALIGN_ERROR(target_align, actual_ptr, example_root_count)                                           \
  if (!target_align.is_aligned(actual_ptr))                                                                           \
  {                                                                                                                   \
    size_t address = reinterpret_cast<size_t>(actual_ptr);                                                            \
    std::stringstream sstream;                                                                                        \
    sstream /* R_E_LS() joins <<s via ',' which makes reading hard; sstream to avoid that */ /* this is fine, because \
                                                                                                we are already about  \
                                                                                                to bail from parsing  \
                                                                                              */                      \
        << "fb_parser error: flatbuffer data not aligned to " << target_align << '\n'                                 \
        << "   example_root[" << example_root_count << "] => @" << std::hex << address << std::dec << " % "           \
        << target_align.align << " = " << address % target_align.align << " (vs desired = " << target_align.offset    \
        << ")";                                                                                                       \
    RETURN_ERROR_LS(status, internal_error) << sstream.str();                                                         \
  }

  using size_prefix_t = uint32_t;
  constexpr std::size_t EXPECTED_ALIGNMENT = 8;  // this is where FB expects the size-prefixed FB to be aligned
  constexpr std::size_t EXPECTED_OFFSET = sizeof(size_prefix_t);  // when we manually read the size-prefix, the data
                                                                  // block of the flat buffer is offset by its size

  desired_align align_prefixed = {EXPECTED_ALIGNMENT, 0};
  desired_align align_data = {EXPECTED_ALIGNMENT, EXPECTED_OFFSET};

  if (buffer_pointer)
  {
    RETURN_IF_ALIGN_ERROR(align_prefixed, buffer_pointer, _num_example_roots);

    _flatbuffer_pointer = buffer_pointer;

    _data = VW::parsers::flatbuffer::GetSizePrefixedExampleRoot(_flatbuffer_pointer);
    _num_example_roots++;
    return VW::experimental::error_code::success;
  }

  char* line = nullptr;
  auto len = buf.buf_read(line, sizeof(size_prefix_t), align_prefixed);  // the prefixed flatbuffer block should be
                                                                         // aligned to 8 bytes, no offset

  if (len < sizeof(uint32_t))
  {
    if (len == 0)
    {
      // nothing to read
      return VW::experimental::error_code::nothing_to_parse;
    }
    else
    {
      // broken file
      RETURN_ERROR_LS(status, internal_error) << "Flatbuffer size prefix is incomplete; input is malformed.";
    }
  }

  _object_size = flatbuffers::ReadScalar<flatbuffers::uoffset_t>(line);

  // read one object, object size defined by the read prefix
  buf.buf_read(line, _object_size, align_data);  // the data block of the flatbuffer should be aligned to 8 bytes,
                                                 // offset by the size of the prefix

  RETURN_IF_ALIGN_ERROR(align_data, line, _num_example_roots);

  _flatbuffer_pointer = reinterpret_cast<uint8_t*>(line);
  _data = VW::parsers::flatbuffer::GetExampleRoot(_flatbuffer_pointer);

  _num_example_roots++;
  return VW::experimental::error_code::success;

#undef RETURN_IF_ALIGN_ERROR
}

int parser::process_collection_item(VW::workspace* all, VW::multi_ex& examples, VW::experimental::api_status* status)
{
  // new example/multi example object to process from collection
  if (_data->example_obj_as_ExampleCollection()->is_multiline())
  {
    _active_multi_ex = true;
    _multi_example_object = _data->example_obj_as_ExampleCollection()->multi_examples()->Get(_example_index);
    RETURN_IF_FAIL(parse_multi_example(all, examples[0], _multi_example_object, status));
    // read from active collection

    if (!_active_multi_ex)
    {
      _example_index++;
      if (_example_index == _data->example_obj_as_ExampleCollection()->multi_examples()->size())
      {
        _example_index = 0;
        _active_collection = false;
      }
    }
  }
  else
  {
    const auto ex = _data->example_obj_as_ExampleCollection()->examples()->Get(_example_index);
    RETURN_IF_FAIL(parse_example(all, examples[0], ex, status));
    _example_index++;
    if (_example_index == _data->example_obj_as_ExampleCollection()->examples()->size())
    {
      _example_index = 0;
      _active_collection = false;
    }
  }
  return VW::experimental::error_code::success;
}

int parser::parse_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples, const uint8_t* buffer_pointer,
    VW::experimental::api_status* status)
{
#define RETURN_SUCCESS_FINISHED() \
  return buffer_pointer ? VW::experimental::error_code::nothing_to_parse : VW::experimental::error_code::success;

  if (_active_collection)
  {
    RETURN_IF_FAIL(process_collection_item(all, examples, status));
    if (!_active_collection) RETURN_SUCCESS_FINISHED();
  }
  else if (_active_multi_ex)
  {
    RETURN_IF_FAIL(parse_multi_example(all, examples[0], _multi_example_object, status));
    if (!_active_multi_ex) RETURN_SUCCESS_FINISHED();
  }
  else
  {
    // new object to be read from file
    RETURN_IF_FAIL(parse(buf, buffer_pointer, status));

    switch (_data->example_obj_type())
    {
      case VW::parsers::flatbuffer::ExampleType_Example:
      {
        const auto example = _data->example_obj_as_Example();
        RETURN_IF_FAIL(parse_example(all, examples[0], example, status));
        RETURN_SUCCESS_FINISHED();
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_MultiExample:
      {
        _multi_example_object = _data->example_obj_as_MultiExample();
        _active_multi_ex = true;

        RETURN_IF_FAIL(parse_multi_example(all, examples[0], _multi_example_object, status));
        if (!_active_multi_ex) RETURN_SUCCESS_FINISHED();
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_ExampleCollection:
      {
        _active_collection = true;

        RETURN_IF_FAIL(process_collection_item(all, examples, status));
        if (!_active_collection) RETURN_SUCCESS_FINISHED();
      }
      break;

      default:
        RETURN_ERROR_LS(status, fb_parser_unknown_example_type) << "Unknown example type";
        break;
    }
  }

  return VW::experimental::error_code::success;
}

int parser::parse_example(VW::workspace* all, example* ae, const Example* eg, VW::experimental::api_status* status)
{
  all->parser_runtime.example_parser->lbl_parser.default_label(ae->l);
  ae->is_newline = eg->is_newline();
  RETURN_IF_FAIL(parse_flat_label(all->sd.get(), ae, eg, all->logger, status));

  if (flatbuffers::IsFieldPresent(eg, Example::VT_TAG))
  {
    VW::string_view tag(eg->tag()->c_str());
    ae->tag.insert(ae->tag.end(), tag.begin(), tag.end());
  }

  // VW::experimental::api_status status;
  for (const auto& ns : *(eg->namespaces())) { RETURN_IF_FAIL(parse_namespaces(all, ae, ns, status)); }
  return VW::experimental::error_code::success;
}

int parser::parse_multi_example(
    VW::workspace* all, example* ae, const MultiExample* eg, VW::experimental::api_status* status)
{
  all->parser_runtime.example_parser->lbl_parser.default_label(ae->l);
  if (_multi_ex_index >= eg->examples()->size())
  {
    // done with multi example, send a newline example and reset
    ae->is_newline = true;
    _multi_ex_index = 0;
    _active_multi_ex = false;
    _multi_example_object = nullptr;
    return VW::experimental::error_code::success;
  }

  RETURN_IF_FAIL(parse_example(all, ae, eg->examples()->Get(_multi_ex_index), status));
  _multi_ex_index++;
  return VW::experimental::error_code::success;
}

int parser::get_namespace_index(const Namespace* ns, namespace_index& ni, VW::experimental::api_status* status)
{
  if (flatbuffers::IsFieldPresent(ns, Namespace::VT_NAME))
  {
    ni = static_cast<uint8_t>(ns->name()->c_str()[0]);
    return VW::experimental::error_code::success;
  }
  else if (flatbuffers::IsFieldPresent(ns, Namespace::VT_HASH))
  {
    ni = ns->hash();
    return VW::experimental::error_code::success;
  }

  if (_active_collection && _active_multi_ex)
  {
    RETURN_ERROR_LS(status, fb_parser_name_hash_missing)
        << "Either name or hash field must be specified to get the namespace index in collection item with example "
           "index "
        << _example_index << "and multi example index " << _multi_ex_index;
  }
  else if (_active_multi_ex)
  {
    RETURN_ERROR_LS(status, fb_parser_name_hash_missing)
        << "Either name or hash field must be specified to get the namespace index in multi example index "
        << _multi_ex_index;
  }
  else
  {
    RETURN_ERROR_LS(status, fb_parser_name_hash_missing)
        << "Either name or hash field must be specified to get the namespace index";
  }
}

bool get_namespace_hash(VW::workspace* all, const Namespace* ns, uint64_t& hash)
{
  if (flatbuffers::IsFieldPresent(ns, Namespace::VT_FULL_HASH))
  {
    hash = ns->full_hash();
    return true;
  }
  else if (flatbuffers::IsFieldPresent(ns, Namespace::VT_NAME))
  {
    hash = all->parser_runtime.example_parser->hasher(
        ns->name()->c_str(), ns->name()->size(), all->runtime_config.hash_seed);
    return true;
  }

  return false;
}

bool features_have_names(const Namespace& ns)
{
  return flatbuffers::IsFieldPresent(&ns, Namespace::VT_FEATURE_NAMES) && (ns.feature_names()->size() != 0);
  // TODO: It is not clear what the right answer is when feature_values->size is 0
}

bool features_have_hashes(const Namespace& ns)
{
  return flatbuffers::IsFieldPresent(&ns, Namespace::VT_FEATURE_HASHES) && (ns.feature_hashes()->size() != 0);
}

bool features_have_values(const Namespace& ns)
{
  return flatbuffers::IsFieldPresent(&ns, Namespace::VT_FEATURE_VALUES) && (ns.feature_values()->size() != 0);
}

int parser::parse_namespaces(VW::workspace* all, example* ae, const Namespace* ns, VW::experimental::api_status* status)
{
#define RETURN_NS_PARSER_ERROR(status, error_code)                                                                 \
  if (_active_collection && _active_multi_ex)                                                                      \
  {                                                                                                                \
    RETURN_ERROR_LS(status, error_code) << "Unable to parse namespace in collection item with example index "      \
                                        << _example_index << "and multi example index " << _multi_ex_index;        \
  }                                                                                                                \
  else if (_active_multi_ex)                                                                                       \
  {                                                                                                                \
    RETURN_ERROR_LS(status, error_code) << "Unable to parse namespace in multi example index " << _multi_ex_index; \
  }                                                                                                                \
  else { RETURN_ERROR_LS(status, error_code) << "Unable to parse namespace "; }

  namespace_index index;
  RETURN_IF_FAIL(parser::get_namespace_index(ns, index, status));
  uint64_t hash = 0;
  const auto hash_found = get_namespace_hash(all, ns, hash);
  if (hash_found) { _c_hash = hash; }
  if (std::find(ae->indices.begin(), ae->indices.end(), index) == ae->indices.end()) { ae->indices.push_back(index); }

  auto& fs = ae->feature_space[index];

  if (hash_found) { fs.start_ns_extent(hash); }

  if (!features_have_values(*ns)) { RETURN_NS_PARSER_ERROR(status, fb_parser_feature_values_missing) }

  auto feature_value_iter = (ns->feature_values())->begin();
  const auto feature_value_iter_end = (ns->feature_values())->end();

  bool has_hashes = features_have_hashes(*ns);
  bool has_names = features_have_names(*ns);

  // assuming the usecase that if feature names would exist, they would exist for all features in the namespace
  if (has_names)
  {
    const auto ns_name = ns->name();
    auto feature_name_iter = (ns->feature_names())->begin();
    if (has_hashes)
    {
      if (ns->feature_hashes()->size() != ns->feature_values()->size())
      {
        RETURN_NS_PARSER_ERROR(status, fb_parser_size_mismatch_ft_hashes_ft_values)
      }

      auto feature_hash_iter = (ns->feature_hashes())->begin();
      for (; feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_hash_iter)
      {
        fs.push_back(*feature_value_iter, *feature_hash_iter);
        if (ns_name != nullptr)
        {
          fs.space_names.emplace_back(audit_strings(ns_name->c_str(), feature_name_iter->c_str()));
          ++feature_name_iter;
        }
      }
    }
    else
    {
      // assuming the usecase that if feature names would exist, they would exist for all features in the namespace
      if (ns->feature_names()->size() != ns->feature_values()->size())
      {
        RETURN_NS_PARSER_ERROR(status, fb_parser_size_mismatch_ft_names_ft_values)
      }
      for (; feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_name_iter)
      {
        const uint64_t word_hash =
            all->parser_runtime.example_parser->hasher(feature_name_iter->c_str(), feature_name_iter->size(), _c_hash) &
            all->runtime_state.parse_mask;
        fs.push_back(*feature_value_iter, word_hash);
        if (ns_name != nullptr)
        {
          fs.space_names.emplace_back(audit_strings(ns_name->c_str(), feature_name_iter->c_str()));
        }
      }
    }
  }
  else
  {
    if (!has_hashes) { RETURN_NS_PARSER_ERROR(status, fb_parser_name_hash_missing) }

    if (ns->feature_hashes()->size() != ns->feature_values()->size())
    {
      RETURN_NS_PARSER_ERROR(status, fb_parser_size_mismatch_ft_hashes_ft_values)
    }

    auto feature_hash_iter = (ns->feature_hashes())->begin();
    for (; feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_hash_iter)
    {
      fs.push_back(*feature_value_iter, *feature_hash_iter);
    }
  }

  if (hash_found) { fs.end_ns_extent(); }

  return VW::experimental::error_code::success;
}

int parser::parse_flat_label(
    shared_data* sd, example* ae, const Example* eg, VW::io::logger& logger, VW::experimental::api_status* status)
{
  switch (eg->label_type())
  {
    case Label_SimpleLabel:
    {
      const SimpleLabel* simple_lbl = static_cast<const SimpleLabel*>(eg->label());
      parse_simple_label(sd, &(ae->l), &(ae->ex_reduction_features), simple_lbl);
      break;
    }
    case Label_CBLabel:
    {
      const CBLabel* cb_label = static_cast<const CBLabel*>(eg->label());
      parse_cb_label(&(ae->l), cb_label);
      break;
    }
    case Label_CCBLabel:
    {
      const CCBLabel* ccb_label = static_cast<const CCBLabel*>(eg->label());
      parse_ccb_label(&(ae->l), ccb_label);
      break;
    }
    case Label_CB_EVAL_Label:
    {
      auto cb_eval_label = static_cast<const CB_EVAL_Label*>(eg->label());
      parse_cb_eval_label(&(ae->l), cb_eval_label);
      break;
    }
    case Label_CS_Label:
    {
      auto cs_label = static_cast<const CS_Label*>(eg->label());
      parse_cs_label(&(ae->l), cs_label);
      break;
    }
    case Label_MultiClass:
    {
      auto mc_label = static_cast<const MultiClass*>(eg->label());
      parse_mc_label(sd, &(ae->l), mc_label, logger);
      break;
    }
    case Label_MultiLabel:
    {
      auto multi_label = static_cast<const MultiLabel*>(eg->label());
      parse_multi_label(&(ae->l), multi_label);
      break;
    }
    case Label_Slates_Label:
    {
      auto slates_label = static_cast<const Slates_Label*>(eg->label());
      RETURN_IF_FAIL(parse_slates_label(&(ae->l), slates_label, nullptr));
      break;
    }
    case Label_ContinuousLabel:
    {
      auto continuous_label = static_cast<const ContinuousLabel*>(eg->label());
      parse_continuous_action_label(&(ae->l), continuous_label);
      break;
    }
    case Label_NONE:
      break;
    default:
      if (_active_collection && _active_multi_ex)
      {
        RETURN_ERROR_LS(status, unknown_label_type) << "Unable to parse label in collection item with example index "
                                                    << _example_index << "and multi example index " << _multi_ex_index;
      }
      else if (_active_multi_ex)
      {
        RETURN_ERROR_LS(status, unknown_label_type)
            << "Unable to parse label in multi example index " << _multi_ex_index;
      }
      else { RETURN_ERROR_LS(status, unknown_label_type) << "Unable to parse label "; }
  }
  return VW::experimental::error_code::success;
}

}  // namespace flatbuffer
}  // namespace parsers
}  // namespace VW
