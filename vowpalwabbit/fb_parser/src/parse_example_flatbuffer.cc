// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/fb_parser/parse_example_flatbuffer.h"

#include "vw/core/action_score.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/global_data.h"
#include "vw/core/parser.h"
#include "vw/core/error_constants.h"

#include <cfloat>
#include <fstream>
#include <iostream>

namespace VW
{
namespace parsers
{
namespace flatbuffer
{
int flatbuffer_to_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples)
{
  VW::experimental::api_status api_status;
  int return_code;
  return static_cast<int>(all->flat_converter->parse_examples(all, buf, examples, return_code));
}

const VW::parsers::flatbuffer::ExampleRoot* parser::data() { return _data; }

bool parser::parse(io_buf& buf, uint8_t* buffer_pointer)
{
  if (buffer_pointer)
  {
    _flatbuffer_pointer = buffer_pointer;

    _data = VW::parsers::flatbuffer::GetSizePrefixedExampleRoot(_flatbuffer_pointer);
    return true;
  }

  char* line = nullptr;
  auto len = buf.buf_read(line, sizeof(uint32_t));

  if (len < sizeof(uint32_t)) { return false; }

  _object_size = flatbuffers::ReadScalar<flatbuffers::uoffset_t>(line);

  // read one object, object size defined by the read prefix
  buf.buf_read(line, _object_size);

  _flatbuffer_pointer = reinterpret_cast<uint8_t*>(line);
  _data = VW::parsers::flatbuffer::GetExampleRoot(_flatbuffer_pointer);
  return true;
}

int parser::process_collection_item(VW::workspace* all, VW::multi_ex& examples, VW::experimental::api_status* status)
{
  // new example/multi example object to process from collection
  if (_data->example_obj_as_ExampleCollection()->is_multiline())
  {
    _active_multi_ex = true;
    _multi_example_object = _data->example_obj_as_ExampleCollection()->multi_examples()->Get(_example_index);
    int return_multi_example = parse_multi_example(all, examples[0], _multi_example_object);
    if (return_multi_example != 0) return return_multi_example;
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
    int return_example = parse_example(all, examples[0], ex);
    if (return_example != 0) return return_example;
    _example_index++;
    if (_example_index == _data->example_obj_as_ExampleCollection()->examples()->size())
    {
      _example_index = 0;
      _active_collection = false;
    }
  }
  return VW::experimental::error_code::success;
}

bool parser::parse_examples(VW::workspace* all, io_buf& buf, VW::multi_ex& examples, int& return_code, uint8_t* buffer_pointer, VW::experimental::api_status* status)
{
  if (_active_multi_ex)
  {
    return_code = parse_multi_example(all, examples[0], _multi_example_object, status);
    return true;
  }

  if (_active_collection)
  {
    return_code = process_collection_item(all, examples, status);
    return true;
  }
  else
  {
    // new object to be read from file
    if (!parse(buf, buffer_pointer)) 
    { 
      return_code = VW::experimental::error_code::fb_parser_failed_to_parse;
      return false; 
    }

    switch (_data->example_obj_type())
    {
      case VW::parsers::flatbuffer::ExampleType_Example:
      {
        const auto example = _data->example_obj_as_Example();
        return_code = parse_example(all, examples[0], example);
        return true;
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_MultiExample:
      {
        _multi_example_object = _data->example_obj_as_MultiExample();
        _active_multi_ex = true;
        return_code = parse_multi_example(all, examples[0], _multi_example_object);
        return true;
      }
      break;
      case VW::parsers::flatbuffer::ExampleType_ExampleCollection:
      {
        _active_collection = true;
        return_code = process_collection_item(all, examples);
        return true;
      }
      break;

      default:
        break;
    }
    return_code = VW::experimental::error_code::fb_parser_unknown_example_type;
    return false;
  }
}

int parser::parse_example(VW::workspace* all, example* ae, const Example* eg, VW::experimental::api_status* status)
{
  all->example_parser->lbl_parser.default_label(ae->l);
  ae->is_newline = eg->is_newline();
  int return_flat_label = parse_flat_label(all->sd, ae, eg, all->logger);
  if (return_flat_label != 0) return return_flat_label;

  if (flatbuffers::IsFieldPresent(eg, Example::VT_TAG))
  {
    VW::string_view tag(eg->tag()->c_str());
    ae->tag.insert(ae->tag.end(), tag.begin(), tag.end());
  }

  // VW::experimental::api_status status;
  int return_namespace;
  for (const auto& ns : *(eg->namespaces())) 
  { 
    int return_error_code_namespace = parse_namespaces(all, ae, ns, nullptr); 
    if(return_error_code_namespace != 0) return return_error_code_namespace;
  }
  return VW::experimental::error_code::success;
}

int parser::parse_multi_example(VW::workspace* all, example* ae, const MultiExample* eg, VW::experimental::api_status* status)
{
  all->example_parser->lbl_parser.default_label(ae->l);
  if (_multi_ex_index >= eg->examples()->size())
  {
    // done with multi example, send a newline example and reset
    ae->is_newline = true;
    _multi_ex_index = 0;
    _active_multi_ex = false;
    _multi_example_object = nullptr;
    return VW::experimental::error_code::success;
  }

  int return_error_code_example = parse_example(all, ae, eg->examples()->Get(_multi_ex_index));
  if (return_error_code_example != 0) return return_error_code_example;
  _multi_ex_index++;
  return VW::experimental::error_code::success;
}

int get_namespace_index(const Namespace* ns, namespace_index& ni, VW::experimental::api_status* status)
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
  
  RETURN_ERROR(status, vw_exception, "Either name or hash field must be specified to get the namespace index.");
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
    hash = all->example_parser->hasher(ns->name()->c_str(), ns->name()->size(), all->hash_seed);
    return true;
  }
  
  return false;
}

int parser::parse_namespaces(VW::workspace* all, example* ae, const Namespace* ns, VW::experimental::api_status* status)
{
  if (!flatbuffers::IsFieldPresent(ns, Namespace::VT_NAME)) { RETURN_ERROR(status, fb_parser_namespace_missing, "namespace is missing"); }
  // if(ns == nullptr) { RETURN_ERROR(status, fb_parser_namespace_missing, "namespace is null"); }
  namespace_index index;
  if(get_namespace_index(ns, index, status) != 0) { RETURN_ERROR(status, vw_exception, "could not get namespace index"); }
  uint64_t hash = 0;
  const auto hash_found = get_namespace_hash(all, ns, hash);
  if (hash_found) { _c_hash = hash; }
  if (std::find(ae->indices.begin(), ae->indices.end(), index) == ae->indices.end()) { ae->indices.push_back(index); }

  auto& fs = ae->feature_space[index];

  if (hash_found) { fs.start_ns_extent(hash); }
  
  if(!flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_VALUES)) { RETURN_ERROR(status, fb_parser_feature_values_missing, "feature values table is null"); }
  // if(ns->feature_values() == nullptr) { RETURN_ERROR(status, fb_parser_feature_values_missing, "feature values table is null"); }

  auto feature_value_iter = (ns->feature_values())->begin();
  const auto feature_value_iter_end = (ns->feature_values())->end();

  //assuming the usecase that if feature names would exist, they would exist for all features in the namespace
  if(flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_NAMES))
  // if(ns->feature_names() != nullptr)
  {
    const auto ns_name = ns->name();
    auto feature_name_iter = (ns->feature_names())->begin();
    if(flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_HASHES))
    // if(ns->feature_hashes() != nullptr)
    {
      auto feature_hash_iter = (ns->feature_hashes())->begin();
      
      for (;feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_hash_iter)
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
      //assuming the usecase that if feature names would exist, they would exist for all features in the namespace
      for (;feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_name_iter)
      {
        const uint64_t word_hash = all->example_parser->hasher(feature_name_iter->c_str(), feature_name_iter->size(), _c_hash);
        fs.push_back(*feature_value_iter, word_hash);
        if (ns_name != nullptr)
        { fs.space_names.emplace_back(audit_strings(ns_name->c_str(), feature_name_iter->c_str())); }
      }
    }
  }
  else
  {
    if(!flatbuffers::IsFieldPresent(ns, Namespace::VT_FEATURE_HASHES)) { RETURN_ERROR(status, fb_parser_feature_hashes_names_missing, "feature hashes table cannot be null for the usecase with feature names null"); }
    // if(ns->feature_hashes() == nullptr) { RETURN_ERROR(status, fb_parser_feature_hashes_names_missing, "feature hashes table cannot be null for the usecase with feature names null"); }
    auto feature_hash_iter = (ns->feature_hashes())->begin();  
    for (;feature_value_iter != feature_value_iter_end; ++feature_value_iter, ++feature_hash_iter)
    {
      fs.push_back(*feature_value_iter, *feature_hash_iter);
    }
  }

  if (hash_found) { fs.end_ns_extent(); }

  return VW::experimental::error_code::success;
}

int parser::parse_flat_label(shared_data* sd, example* ae, const Example* eg, VW::io::logger& logger, VW::experimental::api_status* status)
{
  switch (eg->label_type())
  {
    case Label_SimpleLabel:
    {
      const SimpleLabel* simple_lbl = static_cast<const SimpleLabel*>(eg->label());
      parse_simple_label(sd, &(ae->l), &(ae->_reduction_features), simple_lbl);
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
      return parse_slates_label(&(ae->l), slates_label, nullptr);
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
      RETURN_ERROR(status, not_implemented, "Label type in Flatbuffer not understood"); 
  }
  return VW::experimental::error_code::success;
}

}  // namespace flatbuffer
}  // namespace parsers
}  // namespace VW
