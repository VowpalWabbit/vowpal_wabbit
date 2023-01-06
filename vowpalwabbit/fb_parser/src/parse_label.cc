// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/action_score.h"
#include "vw/core/best_constant.h"
#include "vw/core/cb.h"
#include "vw/core/constant.h"
#include "vw/core/example.h"
#include "vw/core/global_data.h"
#include "vw/core/named_labels.h"
#include "vw/core/slates_label.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"

#include <cfloat>
#include <fstream>
#include <iostream>

namespace VW
{
namespace parsers
{
namespace flatbuffer
{
void parser::parse_simple_label(
    shared_data* /*sd*/, polylabel* l, reduction_features* red_features, const SimpleLabel* label)
{
  auto& simple_red_features = red_features->template get<VW::simple_label_reduction_features>();
  l->simple.label = label->label();
  simple_red_features.weight = label->weight();
  simple_red_features.initial = label->initial();
}

void parser::parse_cb_label(polylabel* l, const CBLabel* label)
{
  l->cb.weight = label->weight();
  for (auto const& cost : *(label->costs()))
  {
    VW::cb_class f;
    f.action = cost->action();
    f.cost = cost->cost();
    f.probability = cost->probability();
    f.partial_prediction = cost->partial_pred();
    l->cb.costs.push_back(f);
  }
}

void parser::parse_ccb_label(polylabel* l, const CCBLabel* label)
{
  l->conditional_contextual_bandit.weight = label->weight();
  if (label->example_type() == 1)
    l->conditional_contextual_bandit.type = VW::ccb_example_type::SHARED;
  else if (label->example_type() == 2)
    l->conditional_contextual_bandit.type = VW::ccb_example_type::ACTION;
  else if (label->example_type() == 3)
  {
    l->conditional_contextual_bandit.type = VW::ccb_example_type::UNSET;

    if (label->explicit_included_actions() != nullptr)
    {
      l->conditional_contextual_bandit.type = VW::ccb_example_type::SLOT;
      for (const auto& exp_included_action : *(label->explicit_included_actions()))
      {
        l->conditional_contextual_bandit.explicit_included_actions.push_back(exp_included_action);
      }
    }

    if (label->outcome() != nullptr)
    {
      l->conditional_contextual_bandit.type = VW::ccb_example_type::SLOT;
      auto& ccb_outcome = *(new VW::ccb_outcome());
      ccb_outcome.cost = label->outcome()->cost();
      ccb_outcome.probabilities.clear();

      for (auto const& as : *(label->outcome()->probabilities()))
        ccb_outcome.probabilities.push_back({as->action(), as->score()});

      l->conditional_contextual_bandit.outcome = &ccb_outcome;
    }
  }
}

void parser::parse_cb_eval_label(polylabel* l, const CB_EVAL_Label* label)
{
  l->cb_eval.action = label->action();
  l->cb_eval.event.weight = label->event()->weight();
  for (const auto& cb_cost : *(label->event()->costs()))
  {
    VW::cb_class f;
    f.cost = cb_cost->cost();
    f.action = cb_cost->action();
    f.probability = cb_cost->probability();
    f.partial_prediction = cb_cost->partial_pred();
    l->cb_eval.event.costs.push_back(f);
  }
}

void parser::parse_cs_label(polylabel* l, const CS_Label* label)
{
  for (auto const& cost : *(label->costs()))
  {
    VW::cs_class f;
    f.x = cost->x();
    f.partial_prediction = cost->partial_pred();
    f.wap_value = cost->wap_value();
    f.class_index = cost->class_index();
    l->cs.costs.push_back(f);
  }
}

void parser::parse_mc_label(shared_data* sd, polylabel* l, const MultiClass* label, VW::io::logger& logger)
{
  std::string named_label;
  if (flatbuffers::IsFieldPresent(label, MultiClass::VT_NAMEDLABEL))
    named_label = std::string(label->namedlabel()->c_str());
  if (sd->ldict)
  {
    if (named_label.empty()) { l->multi.label = static_cast<uint32_t>(-1); }
    else { l->multi.label = static_cast<uint32_t>(sd->ldict->get(VW::string_view(named_label), logger)); }
  }
  else { l->multi.label = label->label(); }
  l->multi.weight = label->weight();
}

void parser::parse_multi_label(polylabel* l, const MultiLabel* label)
{
  for (auto const& lab : *(label->labels())) l->multilabels.label_v.push_back(lab);
}

void parser::parse_slates_label(polylabel* l, const Slates_Label* label)
{
  l->slates.weight = label->weight();
  if (label->example_type() == VW::parsers::flatbuffer::CCB_Slates_example_type::CCB_Slates_example_type_shared)
  {
    l->slates.labeled = label->labeled();
    l->slates.cost = label->cost();
    l->slates.type = VW::slates::example_type::SHARED;
  }
  else if (label->example_type() == VW::parsers::flatbuffer::CCB_Slates_example_type::CCB_Slates_example_type_action)
  {
    l->slates.slot_id = label->slot();
    l->slates.type = VW::slates::example_type::ACTION;
  }
  else if (label->example_type() == VW::parsers::flatbuffer::CCB_Slates_example_type::CCB_Slates_example_type_slot)
  {
    l->slates.labeled = label->labeled();
    l->slates.probabilities.clear();
    l->slates.type = VW::slates::example_type::SLOT;

    for (auto const& as : *(label->probabilities())) l->slates.probabilities.push_back({as->action(), as->score()});
  }
  else { THROW("Example type not understood") }
}

void parser::parse_continuous_action_label(polylabel* l, const VW::parsers::flatbuffer::ContinuousLabel* label)
{
  for (auto const& continuous_element : *(label->costs()))
  {
    l->cb_cont.costs.push_back(
        {continuous_element->action(), continuous_element->cost(), continuous_element->pdf_value()});
  }
}
}  // namespace flatbuffer
}  // namespace parsers
}  // namespace VW
