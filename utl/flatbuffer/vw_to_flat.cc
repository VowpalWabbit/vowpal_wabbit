// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <sys/timeb.h>
#include <fstream>
#include <vector>

#include "vw_to_flat.h"
#include "options.h"
#include "parse_args.h"
#include "parse_regressor.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_exception.h"
#include "options_boost_po.h"

void write_buffer_to_file(std::ofstream& outfile, flatbuffers::FlatBufferBuilder& builder,
    flatbuffers::Offset<VW::parsers::flatbuffer::ExampleRoot>& root)
{
  builder.FinishSizePrefixed(root);
  uint8_t* buf = builder.GetBufferPointer();
  int size = builder.GetSize();
  outfile.write(reinterpret_cast<char*>(buf), size);
}

void to_flat::create_simple_label(
    example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  label = VW::parsers::flatbuffer::CreateSimpleLabel(_builder, v->l.simple.label, v->l.simple.weight).Union();
  label_type = VW::parsers::flatbuffer::Label_SimpleLabel;
}

void to_flat::create_cb_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (auto const& cost : v->l.cb.costs)
  {
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(
        _builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
  }
  label = VW::parsers::flatbuffer::CreateCBLabelDirect(_builder, v->l.cb.weight, &costs).Union();
  label_type = VW::parsers::flatbuffer::Label_CBLabel;
}

void to_flat::create_ccb_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  auto weight = v->l.conditional_contextual_bandit.weight;
  auto e_type = v->l.conditional_contextual_bandit.type;
  label_type = VW::parsers::flatbuffer::Label_CCBLabel;

  if (e_type == CCB::example_type::shared)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
  }
  else if (e_type == CCB::example_type::action)
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
  }
  else if (e_type == CCB::example_type::slot)
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
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, outcome, nullptr).Union();
    }
    else if (&(v->l.conditional_contextual_bandit.explicit_included_actions) != nullptr)
    {
      for (auto const& action : v->l.conditional_contextual_bandit.explicit_included_actions)
      { explicit_included_actions.push_back(action); }
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, &explicit_included_actions).Union();
    }
    else
    {
      label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
    }
  }
  else
  {
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_unset;
    label = VW::parsers::flatbuffer::CreateCCBLabelDirect(_builder, type, 0, nullptr, weight).Union();
  }
}

void to_flat::create_cb_eval_label(
    example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::CB_class>> costs;
  for (const auto& cost : v->l.cb_eval.event.costs)
  {
    costs.push_back(VW::parsers::flatbuffer::CreateCB_class(
        _builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
  }
  auto sub_label = CreateCBLabelDirect(_builder, v->l.cb_eval.event.weight, &costs);
  label = VW::parsers::flatbuffer::CreateCB_EVAL_Label(_builder, v->l.cb_eval.action, sub_label).Union();
  label_type = VW::parsers::flatbuffer::Label_CB_EVAL_Label;
}

void to_flat::create_mc_label(
    VW::named_labels* ldict, example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  if (ldict)
  {
    if (ldict->get(v->l.multi.label).empty())
      label = VW::parsers::flatbuffer::CreateMultiClass(_builder, 0, 0U, v->l.multi.weight).Union();
    else
    {
      VW::string_view named_label = ldict->get(v->l.multi.label);
      label = VW::parsers::flatbuffer::CreateMultiClass(
          _builder, _builder.CreateString(std::string(named_label.begin(), named_label.end())), 0U, v->l.multi.weight)
                  .Union();
    }
  }
  else
  {
    label = VW::parsers::flatbuffer::CreateMultiClass(_builder, 0, v->l.multi.label, v->l.multi.weight).Union();
  }

  label_type = VW::parsers::flatbuffer::Label_MultiClass;
}

void to_flat::create_multi_label(
    example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<uint32_t> labels;
  for (auto const l : v->l.multilabels.label_v) { labels.push_back(l); }

  label = VW::parsers::flatbuffer::CreateMultiLabelDirect(_builder, &labels).Union();
  label_type = VW::parsers::flatbuffer::Label_MultiLabel;
}

void to_flat::create_slates_label(
    example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::action_score>> action_scores;
  float weight = v->l.slates.weight;
  auto e_type = v->l.slates.type;
  label_type = VW::parsers::flatbuffer::Label_Slates_Label;

  if (e_type == 1)
  {  // shared type
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_shared;
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, v->l.slates.labeled, v->l.slates.cost, 0U, nullptr)
                .Union();
  }
  else if (e_type == 2)
  {  // action type
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_action;
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, false, 0.0, v->l.slates.slot_id, nullptr)
                .Union();
  }
  else if (e_type == 3)
  {  // slot type
    auto type = VW::parsers::flatbuffer::CCB_Slates_example_type_slot;
    for (auto const& as : v->l.slates.probabilities)
    { action_scores.push_back(VW::parsers::flatbuffer::Createaction_score(_builder, as.action, as.score)); }
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, type, weight, v->l.slates.labeled, 0.0, 0U, &action_scores)
                .Union();
  }
  else
  {
    label = VW::parsers::flatbuffer::CreateSlates_LabelDirect(
        _builder, VW::parsers::flatbuffer::CCB_Slates_example_type_unset, weight, false, 0.0, 0U, nullptr)
                .Union();
  }
}

void to_flat::create_cs_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::wclass>> costs;
  for (auto const& wc : v->l.cs.costs)
  {
    costs.push_back(
        VW::parsers::flatbuffer::Createwclass(_builder, wc.x, wc.partial_prediction, wc.wap_value, wc.class_index));
  }
  label = VW::parsers::flatbuffer::CreateCS_LabelDirect(_builder, &costs).Union();
  label_type = VW::parsers::flatbuffer::Label_CS_Label;
}

void to_flat::create_no_label(example* v, flatbuffers::Offset<void>& label, VW::parsers::flatbuffer::Label& label_type)
{
  label = VW::parsers::flatbuffer::Createno_label(_builder, (uint8_t)'\000').Union();
}

void to_flat::convert_txt_to_flat(vw& all)
{
  std::ofstream outfile;
  if (output_flatbuffer_name.empty()) { output_flatbuffer_name = all.data_filename + ".fb"; }
  outfile.open(output_flatbuffer_name, std::ios::binary | std::ios::out);
  std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Example>> example_collection;
  size_t collection_count = 0;

  example* ae = all.example_parser->ready_parsed_examples.pop();
  int examples = 0;
  while (ae != nullptr && !ae->end_pass)
  {
    // Create Label for current example
    flatbuffers::Offset<void> label;
    VW::parsers::flatbuffer::Label label_type = VW::parsers::flatbuffer::Label_NONE;
    switch (all.label_type)
    {
      case label_type_t::nolabel:
        to_flat::create_no_label(ae, label, label_type);
        break;
      case label_type_t::cb:
        to_flat::create_cb_label(ae, label, label_type);
        break;
      case label_type_t::ccb:
        to_flat::create_ccb_label(ae, label, label_type);
        break;
      case label_type_t::multi:
        to_flat::create_multi_label(ae, label, label_type);
        break;
      case label_type_t::mc:
        to_flat::create_mc_label(all.sd->ldict, ae, label, label_type);
        break;
      case label_type_t::cs:
        to_flat::create_cs_label(ae, label, label_type);
        break;
      case label_type_t::cb_eval:
        to_flat::create_cb_eval_label(ae, label, label_type);
        break;
      case label_type_t::slates:
        to_flat::create_slates_label(ae, label, label_type);
        break;
      case label_type_t::simple:
        to_flat::create_simple_label(ae, label, label_type);
        break;
      default:
        THROW("Unknown label type");
        break;
    }

    uint64_t multiplier = (uint64_t)all.wpp << all.weights.stride_shift();
    if (multiplier != 1)
    {
      for (features& fs : *ae)
      {
        for (auto& j : fs.indicies) { j /= multiplier; }
      }
    }
    std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>> namespaces;
    for (const namespace_index& ns : ae->indices)
    {
      // Skip over constant namespace as that will be assigned while reading flatbuffer again
      if (ns == 128) { continue; }

      std::vector<flatbuffers::Offset<VW::parsers::flatbuffer::Feature>> fts;

      for (features::iterator& f : ae->feature_space[ns])
      { fts.push_back(VW::parsers::flatbuffer::CreateFeatureDirect(_builder, nullptr, f.value(), f.index())); }
      namespaces.push_back(VW::parsers::flatbuffer::CreateNamespaceDirect(_builder, nullptr, ns, &fts));
    }
    std::string tag(ae->tag.begin(), ae->tag.size());

    if (collection)
    {
      auto flat_namespaces = _builder.CreateVector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>>(namespaces);
      auto flat_example = VW::parsers::flatbuffer::CreateExample(
          _builder, flat_namespaces, label_type, label.Union(), _builder.CreateString(tag.data()));
      example_collection.push_back(flat_example);

      collection_count++;
      if (collection_count >= collection_size)
      {
        auto egcollection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(_builder, &example_collection);
        auto root =
            CreateExampleRoot(_builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, egcollection.Union());
        write_buffer_to_file(outfile, _builder, root);
        _builder.Clear();
        example_collection.clear();
        collection_count = 0;
      }
    }
    else
    {
      auto flat_namespaces = _builder.CreateVector<flatbuffers::Offset<VW::parsers::flatbuffer::Namespace>>(namespaces);
      auto flat_example = VW::parsers::flatbuffer::CreateExample(
          _builder, flat_namespaces, label_type, label.Union(), _builder.CreateString(tag.data()));
      auto root = VW::parsers::flatbuffer::CreateExampleRoot(
          _builder, VW::parsers::flatbuffer::ExampleType_Example, flat_example.Union());
      write_buffer_to_file(outfile, _builder, root);
      _builder.Clear();
    }

    examples++;
    ae = all.example_parser->ready_parsed_examples.pop();
  }

  if (collection && collection_count > 0)
  {
    // left over examples that did not fit in collection
    auto egcollection = VW::parsers::flatbuffer::CreateExampleCollectionDirect(_builder, &example_collection);
    auto root = VW::parsers::flatbuffer::CreateExampleRoot(
        _builder, VW::parsers::flatbuffer::ExampleType_ExampleCollection, egcollection.Union());
    write_buffer_to_file(outfile, _builder, root);
  }

  all.trace_message << "Converted " << examples << " examples" << std::endl;
  all.trace_message << "Flatbuffer " << output_flatbuffer_name << " created" << std::endl;
}