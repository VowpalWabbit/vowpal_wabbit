// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/fb_parser/parse_example_flatbuffer.h"

#include "vw/core/action_score.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/error_constants.h"
#include "vw/core/global_data.h"
#include "vw/core/parser.h"

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
  if (all->parser_runtime.api_status)
  {
    // TODO: At what point do we report the error?
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
  else
  {
    int result = all->parser_runtime.flat_converter->parse_examples(all, buf, examples, nullptr, nullptr);
    switch (result)
    {
      case VW::experimental::error_code::success:
        return 1;
      case VW::experimental::error_code::nothing_to_parse:
        return 0;  // this is not a true error, but indicates that the parser is done
      default:
        THROW("Error parsing examples.");
    }
  }
}

const VW::parsers::flatbuffer::ExampleRoot* parser::data() { return _data; }

int parser::parse(io_buf& buf, uint8_t* buffer_pointer, VW::experimental::api_status* status)
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
                                                                         // aligned to 8 bytse, no offset

  if (len < sizeof(uint32_t))
  {
    if (len == 0)
    {
      // nothing to read
      RETURN_ERROR(status, nothing_to_parse);
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
    _example_index++;
    if (_example_index == _data->example_obj_as_ExampleCollection()->multi_examples()->size())
    {
      _example_index = 0;
      _active_collection = false;
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

int parser::parse_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples, uint8_t* buffer_pointer,
    VW::experimental::api_status* status)
{
  if (_active_multi_ex) { RETURN_IF_FAIL(parse_multi_example(all, examples[0], _multi_example_object, status)); }
  else if (_active_collection) { RETURN_IF_FAIL(process_collection_item(all, examples, status)); }
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
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_MultiExample:
      {
        _multi_example_object = _data->example_obj_as_MultiExample();
        _active_multi_ex = true;
        RETURN_IF_FAIL(parse_multi_example(all, examples[0], _multi_example_object, status));
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_ExampleCollection:
      {
        _active_collection = true;
        RETURN_IF_FAIL(process_collection_item(all, examples, status));
      }
      break;

      default:
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

int parser::parse_namespaces(VW::workspace* all, example* ae, const Namespace* ns, VW::experimental::api_status* status)
{
  namespace_index index;
  RETURN_IF_FAIL(parser::get_namespace_index(ns, index, status));
  uint64_t hash = 0;
  const auto hash_found = get_namespace_hash(all, ns, hash);
  if (hash_found) { _c_hash = hash; }
  if (std::find(ae->indices.begin(), ae->indices.end(), index) == ae->indices.end()) { ae->indices.push_back(index); }

  auto& fs = ae->feature_space[index];

  if (hash_found) { fs.start_ns_extent(hash); }

  if (!flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_VALUES))
  {
    if (_active_collection && _active_multi_ex)
    {
      RETURN_ERROR_LS(status, fb_parser_feature_values_missing)
          << "Unable to parse namespace in collection item with example index " << _example_index
          << "and multi example index " << _multi_ex_index;
    }
    else if (_active_multi_ex)
    {
      RETURN_ERROR_LS(status, fb_parser_feature_values_missing)
          << "Unable to parse namespace in multi example index " << _multi_ex_index;
    }
    else { RETURN_ERROR_LS(status, fb_parser_feature_values_missing) << "Unable to parse namespace "; }
  }

  auto feature_value_iter = (ns->feature_values())->begin();
  const auto feature_value_iter_end = (ns->feature_values())->end();

  // assuming the usecase that if feature names would exist, they would exist for all features in the namespace
  if (flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_NAMES))
  {
    const auto ns_name = ns->name();
    auto feature_name_iter = (ns->feature_names())->begin();
    if (flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_HASHES))
    {
      if (ns->feature_hashes()->size() != ns->feature_values()->size())
      {
        if (_active_collection && _active_multi_ex)
        {
          RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values)
              << "Unable to parse namespace in collection item with example index " << _example_index
              << "and multi example index " << _multi_ex_index;
        }
        else if (_active_multi_ex)
        {
          RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values)
              << "Unable to parse namespace in multi example index " << _multi_ex_index;
        }
        else { RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values) << "Unable to parse namespace "; }
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
        if (_active_collection && _active_multi_ex)
        {
          RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_names_ft_values)
              << "Unable to parse namespace in collection item with example index " << _example_index
              << "and multi example index " << _multi_ex_index;
        }
        else if (_active_multi_ex)
        {
          RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_names_ft_values)
              << "Unable to parse namespace in multi example index " << _multi_ex_index;
        }
        else { RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_names_ft_values) << "Unable to parse namespace "; }
      }
      for (; feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_name_iter)
      {
        const uint64_t word_hash =
            all->parser_runtime.example_parser->hasher(feature_name_iter->c_str(), feature_name_iter->size(), _c_hash);
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
    if (!flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_HASHES))
    {
      if (_active_collection && _active_multi_ex)
      {
        RETURN_ERROR_LS(status, fb_parser_feature_hashes_names_missing)
            << "Unable to parse namespace in collection item with example index " << _example_index
            << "and multi example index " << _multi_ex_index;
      }
      else if (_active_multi_ex)
      {
        RETURN_ERROR_LS(status, fb_parser_feature_hashes_names_missing)
            << "Unable to parse namespace in multi example index " << _multi_ex_index;
      }
      else { RETURN_ERROR_LS(status, fb_parser_feature_hashes_names_missing) << "Unable to parse namespace "; }
    }
    if (ns->feature_hashes()->size() != ns->feature_values()->size())
    {
      if (_active_collection && _active_multi_ex)
      {
        RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values)
            << "Unable to parse namespace in collection item with example index " << _example_index
            << "and multi example index " << _multi_ex_index;
      }
      else if (_active_multi_ex)
      {
        RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values)
            << "Unable to parse namespace in multi example index " << _multi_ex_index;
      }
      else { RETURN_ERROR_LS(status, fb_parser_size_mismatch_ft_hashes_ft_values) << "Unable to parse namespace "; }
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
