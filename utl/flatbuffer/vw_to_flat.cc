// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_to_flat.h"

#include "vw/common/hash.h"
#include "vw/common/vw_exception.h"
#include "vw/core/accumulate.h"
#include "vw/core/best_constant.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_regressor.h"
#include "vw/core/reductions/cb/cb_algs.h"
#include "vw/core/shared_data.h"

#include <sys/timeb.h>

#include <fstream>
#include <vector>

void write_buffer_to_file(std::ofstream& outfile, flatbuffers::FlatBufferBuilder& builder,
    flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot>& root)
{
  builder.FinishSizePrefixed(root);
  uint8_t* buf = builder.GetBufferPointer();
  int size = builder.GetSize();
  builder.Clear();
  outfile.write(reinterpret_cast<char*>(buf), size);
}

void to_flat::write_collection_to_file(bool is_multiline, std::ofstream& outfile)
{
  flatbuffers::Offset<VW::parsers::flatbuffer::ExampleCollection> egcollection;
  if (is_multiline)
  {
    egcollection =
        VW::parsers::flatbuffer::CreateExampleCollectionDirect(_builder, nullptr, &_multi_example_collection, true);
    _multi_example_collection.clear();
  }
  else
  {
    egcollection =
        VW::parsers::flatbuffer::CreateExampleCollectionDirect(_builder, &_example_collection, nullptr, false);
    _example_collection.clear();
  }
  auto root = CreateExampleRoot(_builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, egcollection.Union());
  write_buffer_to_file(outfile, _builder, root);
}

void to_flat::write_to_file(bool collection, bool is_multiline, MultiExampleBuilder& multi_ex_builder,
    ExampleBuilder& ex_builder, std::ofstream& outfile)
{
  if (collection)
  {
    if (is_multiline)
    {
      auto multi_ex = multi_ex_builder.to_flat_example(_builder);
      _multi_example_collection.push_back(multi_ex);
      _multi_ex_index = 0;
    }
    else
    {
      auto ex = ex_builder.to_flat_example(_builder);
      _example_collection.push_back(ex);
    }
    _collection_count++;
    if (_collection_count >= collection_size)
    {
      write_collection_to_file(is_multiline, outfile);
      _collection_count = 0;
      _share_examples.clear();  // can't share between collections
    }
  }
  else
  {
    _share_examples.clear();  // not in collection mode, there is not sharing
    if (is_multiline)
    {
      auto multi_ex = multi_ex_builder.to_flat_example(_builder);
      _multi_ex_index = 0;

      auto root = VW::parsers::flatbuffer::CreateExampleRoot(
          _builder, VW::parsers::flatbuffer::ExampleType_MultiExample, multi_ex.Union());
      write_buffer_to_file(outfile, _builder, root);
    }
    else
    {
      auto ex = ex_builder.to_flat_example(_builder);

      auto root = VW::parsers::flatbuffer::CreateExampleRoot(
          _builder, VW::parsers::flatbuffer::ExampleType_Example, ex.Union());
      write_buffer_to_file(outfile, _builder, root);
    }
  }
}

void to_flat::create_simple_label(VW::example* v, ExampleBuilder& ex_builder)
{
  const auto& red_features = v->ex_reduction_features.template get<VW::simple_label_reduction_features>();
  ex_builder.label =
      VW::parsers::flatbuffer::CreateSimpleLabel(_builder, v->l.simple.label, red_features.weight, red_features.initial)
          .Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_SimpleLabel;
}

void to_flat::create_continuous_action_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Continuous_Label_Elm>> costs;
  for (const auto& continuous_element : v->l.cb_cont.costs)
  {
    costs.push_back(VW::parsers::flatbuffer::CreateContinuous_Label_Elm(
        _builder, continuous_element.action, continuous_element.cost, continuous_element.pdf_value));
  }
  ex_builder.label =
      VW::parsers::flatbuffer::CreateContinuousLabelDirect(_builder, costs.empty() ? nullptr : &costs).Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_ContinuousLabel;
}

void to_flat::create_cb_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (auto const& cost : v->l.cb.costs)
  {
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(
        _builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
  }
  ex_builder.label = VW::parsers::flatbuffer::CreateCBLabelDirect(_builder, v->l.cb.weight, &costs).Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_CBLabel;
}

void to_flat::create_ccb_label(VW::example* v, ExampleBuilder& ex_builder)
{
  auto weight = v->l.conditional_contextual_bandit.weight;
  auto e_type = v->l.conditional_contextual_bandit.type;

  if (e_type == VW::ccb_example_type::SHARED)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    ex_builder.label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
    ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
  }
  else if (e_type == VW::ccb_example_type::ACTION)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    ex_builder.label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
    ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
  }
  else if (e_type == VW::ccb_example_type::SLOT)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
    std::vector<uint32_t> explicit_included_actions;
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
    if (v->l.conditional_contextual_bandit.outcome != nullptr)
    {
      for (const auto& probability : v->l.conditional_contextual_bandit.outcome->probabilities)
      {
        auto action = probability.action;
        auto score = probability.score;
        action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(_builder, action, score));
      }
      auto cost = v->l.conditional_contextual_bandit.outcome->cost;
      auto outcome = VW::parsers::flatbuffer::CreateCCB_outcomeDirect(_builder, cost, &action_scores);
      if (&(v->l.conditional_contextual_bandit.explicit_included_actions) != nullptr)
      {
        for (auto const& action : v->l.conditional_contextual_bandit.explicit_included_actions)
        {
          explicit_included_actions.push_back(action);
        }
      }
      ex_builder.label =
          VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, outcome, &explicit_included_actions).Union();
      ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
    }
    else if (&(v->l.conditional_contextual_bandit.explicit_included_actions) != nullptr)
    {
      for (auto const& action : v->l.conditional_contextual_bandit.explicit_included_actions)
      {
        explicit_included_actions.push_back(action);
      }
      ex_builder.label =
          VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, &explicit_included_actions).Union();
      ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
    }
    else
    {
      ex_builder.label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
      ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
    }
  }
  else
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_unset;
    ex_builder.label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
    ex_builder.label_type = VW::parsers::flatbuffer::Label_CCBLabel;
  }
}

void to_flat::create_cb_eval_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (const auto& cost : v->l.cb_eval.event.costs)
  {
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(
        _builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
  }
  auto sub_label = CreateCBLabelDirect(_builder, v->l.cb_eval.event.weight, &costs);
  ex_builder.label = VW::parsers::flatbuffer::CreateCB_EVAL_Label(_builder, v->l.cb_eval.action, sub_label).Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_CB_EVAL_Label;
}

void to_flat::create_mc_label(VW::named_labels* ldict, VW::example* v, ExampleBuilder& ex_builder)
{
  if (ldict)
  {
    if (ldict->get(v->l.multi.label).empty())
    {
      ex_builder.label = VW::parsers::flatbuffer::CreateMultiClass(_builder, 0, 0U, v->l.multi.weight).Union();
    }
    else
    {
      VW::string_view named_label = ldict->get(v->l.multi.label);
      ex_builder.label = VW::parsers::flatbuffer::CreateMultiClass(
          _builder, _builder.CreateString(std::string(named_label.begin(), named_label.end())), 0U, v->l.multi.weight)
                             .Union();
    }
  }
  else
  {
    ex_builder.label =
        VW::parsers::flatbuffer::CreateMultiClass(_builder, 0, v->l.multi.label, v->l.multi.weight).Union();
  }

  ex_builder.label_type = VW::parsers::flatbuffer::Label_MultiClass;
}

void to_flat::create_multi_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<uint32_t> labels;
  for (auto const l : v->l.multilabels.label_v) { labels.push_back(l); }

  ex_builder.label = VW::parsers::flatbuffer::CreateMultiLabelDirect(_builder, &labels).Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_MultiLabel;
}

void to_flat::create_slates_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
  float weight = v->l.slates.weight;
  auto e_type = v->l.slates.type;
  ex_builder.label_type = VW::parsers::flatbuffer::Label_Slates_Label;

  if (e_type == VW::slates::example_type::SHARED)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    ex_builder.label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, v->l.slates.labeled, v->l.slates.cost, 0U, nullptr)
                           .Union();
  }
  else if (e_type == VW::slates::example_type::ACTION)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    ex_builder.label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, false, 0.0, v->l.slates.slot_id, nullptr)
                           .Union();
  }
  else if (e_type == VW::slates::example_type::SLOT)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
    for (auto const& as : v->l.slates.probabilities)
    {
      action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(_builder, as.action, as.score));
    }
    ex_builder.label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, v->l.slates.labeled, 0.0, 0U, &action_scores)
                           .Union();
  }
  else
  {
    ex_builder.label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, VW::parsers::flatbuffer::CCB_Slates_example_type_unset, weight, false, 0.0, 0U, nullptr)
                           .Union();
  }
}

void to_flat::create_cs_label(VW::example* v, ExampleBuilder& ex_builder)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::wclass>> costs;
  for (auto const& wc : v->l.cs.costs)
  {
    costs.push_back(
        VW::parsers::flatbuffer::Createwclass(_builder, wc.x, wc.partial_prediction, wc.wap_value, wc.class_index));
  }
  ex_builder.label = VW::parsers::flatbuffer::CreateCS_LabelDirect(_builder, &costs).Union();
  ex_builder.label_type = VW::parsers::flatbuffer::Label_CS_Label;
}

void to_flat::create_no_label(VW::example* v, ExampleBuilder& ex_builder)
{
  ex_builder.label = VW::parsers::flatbuffer::Createno_label(_builder, (uint8_t)'\000').Union();
}

//Create namespace when audit is true
flatbuffers::Offset<VW::parsers::flatbuffer::Namespace> to_flat::create_namespace_audit(
    VW::features::audit_iterator begin, VW::features::audit_iterator end, VW::namespace_index index, uint64_t hash)
{
  std::stringstream ss;
  ss << index;

  for (auto it = begin; it != end; ++it) { ss << it.index() << it.value(); }
  ss << ":" << hash;

  std::string s = ss.str();
  uint64_t refid = VW::uniform_hash(s.c_str(), s.size(), 0);
  const auto find_ns_offset = _share_examples.find(refid);

  if (find_ns_offset == _share_examples.end())
  {
    flatbuffers::Offset<VW::parsers::flatbuffer::Namespace> namespace_offset;
    std::vector<flatbuffers::Offset<flatbuffers::String>> feature_names;
    std::vector<float> feature_values;
    std::vector<uint64_t> feature_hashes; 

    // new namespace

    std::string ns_name;
    for (auto it = begin; it != end; ++it)
    {
      if( (it.audit()->ns).c_str() != nullptr ) ns_name = it.audit()->ns;
      
      (feature_names).push_back(_builder.CreateString(it.audit()->name.c_str()));
      (feature_values).push_back(it.value());
      (feature_hashes).push_back(it.index());
    }
    namespace_offset = VW::parsers::flatbuffer::CreateNamespaceDirect(_builder, ns_name.c_str(), index, hash, &feature_names, &feature_values, &feature_hashes);

    _share_examples[refid] = namespace_offset;
  }

  return _share_examples[refid];
}

//Create namespace when audit is false
flatbuffers::Offset<VW::parsers::flatbuffer::Namespace> to_flat::create_namespace(
    features::const_iterator begin, features::const_iterator end, VW::namespace_index index, uint64_t hash)
{
  std::stringstream ss;
  ss << index;

  for (auto it = begin; it != end; ++it) { ss << it.index() << it.value(); }
  ss << ":" << hash;

  std::string s = ss.str();
  uint64_t refid = VW::uniform_hash(s.c_str(), s.size(), 0);
  const auto find_ns_offset = _share_examples.find(refid);

  if (find_ns_offset == _share_examples.end())
  {
    flatbuffers::Offset<VW::parsers::flatbuffer::Namespace> namespace_offset;
    std::vector<float> feature_values;
    std::vector<uint64_t> feature_hashes; 

    for (auto it = begin; it != end; ++it)
    {
      if(it.value() !=0) //store the feature data only if the value is non zero
      {
        (feature_values).push_back(it.value());
        (feature_hashes).push_back(it.index());
      }
    }
    namespace_offset = VW::parsers::flatbuffer::CreateNamespaceDirect(_builder, nullptr, index, hash, nullptr, &feature_values, &feature_hashes);
  
    _share_examples[refid] = namespace_offset;
  }

  return _share_examples[refid];
}

std::vector<VW::namespace_extent> unflatten_namespace_extents_dont_skip(
    const std::vector<std::pair<bool, uint64_t>>& extents)
{
  if (extents.empty()) { return {}; }
  std::vector<VW::namespace_extent> results;
  auto last_start = std::size_t{0};
  auto current = extents[0];
  for (auto i = std::size_t{1}; i < extents.size(); ++i)
  {
    if (current != extents[i])
    {
      // Check if it was a valid sequence, or an empty segment.
      results.emplace_back(last_start, i, current.second);
      last_start = i;
      current = extents[i];
    }
  }

  results.emplace_back(last_start, extents.size(), current.second);
  return results;
}

void to_flat::convert_txt_to_flat(VW::workspace& all)
{
  std::ofstream outfile;
  if (output_flatbuffer_name.empty()) { output_flatbuffer_name = all.parser_runtime.data_filename + ".fb"; }
  outfile.open(output_flatbuffer_name, std::ios::binary | std::ios::out);

  MultiExampleBuilder multi_ex_builder;
  ExampleBuilder ex_builder;

  VW::example* ae = nullptr;
  all.parser_runtime.example_parser->ready_parsed_examples.try_pop(ae);

  while (ae != nullptr && !ae->end_pass)
  {
    // Create Label for current example
    flatbuffers::Offset<void> label;
    VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
    switch (all.parser_runtime.example_parser->lbl_parser.label_type)
    {
      case VW::label_type_t::NOLABEL:
        to_flat::create_no_label(ae, ex_builder);
        break;
      case VW::label_type_t::CB:
        to_flat::create_cb_label(ae, ex_builder);
        break;
      case VW::label_type_t::CCB:
        to_flat::create_ccb_label(ae, ex_builder);
        break;
      case VW::label_type_t::MULTILABEL:
        to_flat::create_multi_label(ae, ex_builder);
        break;
      case VW::label_type_t::MULTICLASS:
        to_flat::create_mc_label(all.sd->ldict.get(), ae, ex_builder);
        break;
      case VW::label_type_t::CS:
        to_flat::create_cs_label(ae, ex_builder);
        break;
      case VW::label_type_t::CB_EVAL:
        to_flat::create_cb_eval_label(ae, ex_builder);
        break;
      case VW::label_type_t::SLATES:
        to_flat::create_slates_label(ae, ex_builder);
        break;
      case VW::label_type_t::SIMPLE:
        to_flat::create_simple_label(ae, ex_builder);
        break;
      case VW::label_type_t::CONTINUOUS:
        to_flat::create_continuous_action_label(ae, ex_builder);
        break;
      default:
        THROW("label_type has not been set or is unknown");
        break;
    }

    uint64_t multiplier = (uint64_t)all.reduction_state.total_feature_width << all.weights.stride_shift();
    if (multiplier != 1)
    {
      for (VW::features& fs : *ae)
      {
        for (auto& j : fs.indices) { j /= multiplier; }
      }
    }
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
    for (const VW::namespace_index& ns : ae->indices)
    {
      // Skip over constant namespace as that will be assigned while reading flatbuffer again
      if (ns == 128) { continue; }

      // Not all features exist in an extent - so we reform the extents list to one which contains non-hash-extents so
      // we can add all of these segments.
      auto flattened_extents =
          VW::details::flatten_namespace_extents(ae->feature_space[ns].namespace_extents, ae->feature_space[ns].size());
      auto unflattened_with_ranges_that_dont_have_extents = unflatten_namespace_extents_dont_skip(flattened_extents);

      if( all.output_config.audit || all.output_config.hash_inv )
      {
        for (const auto& extent : unflattened_with_ranges_that_dont_have_extents)
        {
          // The extent hash for a non-hash-extent will be 0, which is the same as the field no existing to flatbuffers.
          auto created_ns = create_namespace_audit(ae->feature_space[ns].audit_begin() + extent.begin_index,
              ae->feature_space[ns].audit_begin() + extent.end_index, ns, extent.hash);
          namespaces.push_back(created_ns);
        }
      }
      else
      {
        for (const auto& extent : unflattened_with_ranges_that_dont_have_extents)
        {
          // The extent hash for a non-hash-extent will be 0, which is the same as the field no existing to flatbuffers.
          auto created_ns = create_namespace(ae->feature_space[ns].cbegin() + extent.begin_index,
              ae->feature_space[ns].cbegin() + extent.end_index, ns, extent.hash);
          namespaces.push_back(created_ns);
        }
      }
      
    }
    std::string tag(ae->tag.begin(), ae->tag.size());

    if (all.l->is_multiline())
    {
      if (!VW::example_is_newline(*ae) ||
          (all.parser_runtime.example_parser->lbl_parser.label_type == VW::label_type_t::CB &&
              !VW::example_is_newline_not_header_cb(*ae)) ||
          ((all.parser_runtime.example_parser->lbl_parser.label_type == VW::label_type_t::CCB &&
               ae->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT) ||
              (all.parser_runtime.example_parser->lbl_parser.label_type == VW::label_type_t::SLATES &&
                  ae->l.slates.type == VW::slates::example_type::SLOT)))
      {
        ex_builder.namespaces.insert(ex_builder.namespaces.end(), namespaces.begin(), namespaces.end());
        ex_builder.is_newline = ae->is_newline;
        ex_builder.tag = tag;
        multi_ex_builder.examples.push_back(ex_builder);
        ex_builder.clear();
        _multi_ex_index++;
        _examples++;
        ae = nullptr;
        all.parser_runtime.example_parser->ready_parsed_examples.try_pop(ae);
        continue;
      }
      else { ex_builder.is_newline = true; }
    }
    else
    {
      ex_builder.namespaces.insert(ex_builder.namespaces.end(), namespaces.begin(), namespaces.end());
      ex_builder.is_newline = ae->is_newline;
      ex_builder.tag = tag;
      _examples++;
    }

    write_to_file(collection, all.l->is_multiline(), multi_ex_builder, ex_builder, outfile);

    ae = nullptr;
    all.parser_runtime.example_parser->ready_parsed_examples.try_pop(ae);
  }

  if (collection && _collection_count > 0)
  {
    // left over examples that did not fit in collection
    write_collection_to_file(all.l->is_multiline(), outfile);
  }
  else if (all.l->is_multiline() && _multi_ex_index > 0)
  {
    // left over multi examples at end of file
    write_to_file(collection, all.l->is_multiline(), multi_ex_builder, ex_builder, outfile);
  }

  *(all.output_runtime.trace_message) << "Converted " << _examples << " examples" << std::endl;
  *(all.output_runtime.trace_message) << "Flatbuffer " << output_flatbuffer_name << " created" << std::endl;
}
