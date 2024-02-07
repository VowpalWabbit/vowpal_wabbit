// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "prototype_label.h"

#include "vw/core/cb_continuous_label.h"
#include "vw/core/slates_label.h"

namespace vwtest
{
Offset<void> prototype_label_t::create_flatbuffer(FlatBufferBuilder& builder, VW::workspace&) const
{
  {
    switch (label_type)
    {
      case fb::Label_SimpleLabel:
      {
        auto red_features = reduction_features.get<VW::simple_label_reduction_features>();
        return VW::parsers::flatbuffer::CreateSimpleLabel(
            builder, label.simple.label, red_features.weight, red_features.initial)
            .Union();
      }
      case fb::Label_CBLabel:
      {
        std::vector<flatbuffers::Offset<fb::CB_class>> action_costs;
        for (const auto& cost : label.cb.costs)
        {
          action_costs.push_back(
              fb::CreateCB_class(builder, cost.cost, cost.action, cost.probability, cost.partial_prediction));
        }

        Offset<Vector<Offset<fb::CB_class>>> action_costs_vector = builder.CreateVector(action_costs);

        return fb::CreateCBLabel(builder, label.cb.weight, action_costs_vector).Union();
      }
      case fb::Label_NONE:
      {
        return 0;
      }
      case fb::Label_ContinuousLabel:
      {
        std::vector<Offset<fb::Continuous_Label_Elm>> costs;
        costs.reserve(label.cb_cont.costs.size());

        for (const auto& cost : label.cb_cont.costs)
        {
          costs.push_back(fb::CreateContinuous_Label_Elm(builder, cost.action, cost.cost));
        }

        Offset<Vector<Offset<fb::Continuous_Label_Elm>>> costs_fb_vector = builder.CreateVector(costs);
        return fb::CreateContinuousLabel(builder, costs_fb_vector).Union();
      }
      case fb::Label_Slates_Label:
      {
        fb::CCB_Slates_example_type example_type = fb::CCB_Slates_example_type::CCB_Slates_example_type_unset;
        switch (label.slates.type)
        {
          case VW::slates::example_type::UNSET:
            example_type = fb::CCB_Slates_example_type::CCB_Slates_example_type_unset;
            break;
          case VW::slates::example_type::ACTION:
            example_type = fb::CCB_Slates_example_type::CCB_Slates_example_type_action;
            break;
          case VW::slates::example_type::SHARED:
            example_type = fb::CCB_Slates_example_type::CCB_Slates_example_type_shared;
            break;
          case VW::slates::example_type::SLOT:
            example_type = fb::CCB_Slates_example_type::CCB_Slates_example_type_slot;
            break;
          default:
            THROW("Slate label example type not currently supported");
        }

        auto action_scores = label.slates.probabilities;

        // TODO: This conversion is kind of painful: we should consider expanding the probabilities
        // vector into a pair of vectors
        std::vector<Offset<fb::action_score>> fb_action_scores;
        fb_action_scores.reserve(action_scores.size());
        for (const auto& action_score : action_scores)
        {
          fb_action_scores.push_back(fb::Createaction_score(builder, action_score.action, action_score.score));
        }

        Offset<Vector<Offset<fb::action_score>>> fb_action_scores_fb_vector = builder.CreateVector(fb_action_scores);

        return fb::CreateSlates_Label(builder, example_type, label.slates.weight, label.slates.labeled,
            label.slates.cost, label.slates.slot_id, fb_action_scores_fb_vector)
            .Union();
      }
      default:
      {
        THROW("Label type not currently supported for create_flatbuffer");
        return 0;
      }
    }
  }
}

void prototype_label_t::verify(VW::workspace&, const fb::Example* e) const
{
  switch (label_type)
  {
    case fb::Label_SimpleLabel:
    {
      verify_simple_label(e);
      break;
    }
    case fb::Label_CBLabel:
    {
      verify_cb_label(e);
      break;
    }
    case fb::Label_NONE:
    {
      break;
    }
    case fb::Label_ContinuousLabel:
    {
      verify_continuous_label(e);
      break;
    }
    case fb::Label_Slates_Label:
    {
      verify_slates_label(e);
      break;
    }
    // TODO: other label types
    default:
    {
      THROW("Label type not currently supported for verify");
      break;
    }
  }
}

void prototype_label_t::verify(VW::workspace&, const VW::example& e) const
{
  switch (label_type)
  {
    case fb::Label_SimpleLabel:
    {
      verify_simple_label(e);
      break;
    }
    case fb::Label_CBLabel:
    {
      verify_cb_label(e);
      break;
    }
    case fb::Label_NONE:
    {
      break;
    }
    case fb::Label_ContinuousLabel:
    {
      verify_continuous_label(e);
      break;
    }
    case fb::Label_Slates_Label:
    {
      verify_slates_label(e);
      break;
    }
    default:
    {
      THROW("Label type not currently supported for verify");
      break;
    }
  }
}

void prototype_label_t::verify(VW::workspace&, fb::Label label_type, const void* label) const
{
  switch (label_type)
  {
    case fb::Label_SimpleLabel:
    {
      verify_simple_label(GetRoot<fb::SimpleLabel>(label));
      break;
    }
    case fb::Label_CBLabel:
    {
      verify_cb_label(GetRoot<fb::CBLabel>(label));
      break;
    }
    case fb::Label_NONE:
    {
      EXPECT_EQ(label, nullptr);
      break;
    }
    case fb::Label_ContinuousLabel:
    {
      verify_continuous_label(GetRoot<fb::ContinuousLabel>(label));
      break;
    }
    case fb::Label_Slates_Label:
    {
      verify_slates_label(GetRoot<fb::Slates_Label>(label));
      break;
    }
    default:
    {
      THROW("Label type not currently supported for verify");
      break;
    }
  }
}

void prototype_label_t::verify_simple_label(const fb::SimpleLabel* actual_label) const
{
  const auto expected_reduction_features = reduction_features.get<VW::simple_label_reduction_features>();

  EXPECT_FLOAT_EQ(actual_label->label(), label.simple.label);
  EXPECT_FLOAT_EQ(actual_label->initial(), expected_reduction_features.initial);
  EXPECT_FLOAT_EQ(actual_label->weight(), expected_reduction_features.weight);
}

void prototype_label_t::verify_simple_label(const VW::example& e) const
{
  using label_t = VW::simple_label;
  using reduction_features_t = VW::simple_label_reduction_features;

  const label_t actual_label = e.l.simple;
  const reduction_features_t actual_reduction_features = e.ex_reduction_features.template get<reduction_features_t>();
  const reduction_features_t expected_reduction_features = reduction_features.template get<reduction_features_t>();

  EXPECT_FLOAT_EQ(actual_label.label, label.simple.label);
  EXPECT_FLOAT_EQ(actual_reduction_features.initial, expected_reduction_features.initial);
  EXPECT_FLOAT_EQ(actual_reduction_features.weight, expected_reduction_features.weight);
}

void prototype_label_t::verify_cb_label(const fb::CBLabel* actual_label) const
{
  EXPECT_FLOAT_EQ(actual_label->weight(), label.cb.weight);
  EXPECT_EQ(actual_label->costs()->size(), label.cb.costs.size());

  for (size_t i = 0; i < actual_label->costs()->size(); i++)
  {
    auto actual_cost = actual_label->costs()->Get(i);
    auto expected_cost = label.cb.costs[i];

    EXPECT_EQ(actual_cost->action(), expected_cost.action);
    EXPECT_FLOAT_EQ(actual_cost->cost(), expected_cost.cost);
    EXPECT_FLOAT_EQ(actual_cost->probability(), expected_cost.probability);
  }
}

void prototype_label_t::verify_cb_label(const VW::example& e) const
{
  using label_t = VW::cb_label;

  const label_t actual_label = e.l.cb;

  EXPECT_EQ(actual_label.weight, label.cb.weight);
  EXPECT_EQ(actual_label.costs.size(), label.cb.costs.size());

  for (size_t i = 0; i < actual_label.costs.size(); i++)
  {
    EXPECT_EQ(actual_label.costs[i].action, label.cb.costs[i].action);
    EXPECT_FLOAT_EQ(actual_label.costs[i].cost, label.cb.costs[i].cost);
    EXPECT_FLOAT_EQ(actual_label.costs[i].probability, label.cb.costs[i].probability);
  }
}

void prototype_label_t::verify_continuous_label(const fb::ContinuousLabel* actual_label) const
{
  EXPECT_FLOAT_EQ(actual_label->costs()->size(), label.cb_cont.costs.size());

  for (size_t i = 0; i < actual_label->costs()->size(); i++)
  {
    auto actual_cost = actual_label->costs()->Get(i);
    auto expected_cost = label.cb_cont.costs[i];

    EXPECT_EQ(actual_cost->action(), expected_cost.action);
    EXPECT_FLOAT_EQ(actual_cost->cost(), expected_cost.cost);
  }
}

void prototype_label_t::verify_continuous_label(const VW::example& e) const
{
  using label_t = VW::cb_continuous::continuous_label;

  const label_t actual_label = e.l.cb_cont;

  EXPECT_EQ(actual_label.costs.size(), label.cb_cont.costs.size());

  for (size_t i = 0; i < actual_label.costs.size(); i++)
  {
    EXPECT_EQ(actual_label.costs[i].action, label.cb_cont.costs[i].action);
    EXPECT_FLOAT_EQ(actual_label.costs[i].cost, label.cb_cont.costs[i].cost);
  }
}

bool are_equal(fb::CCB_Slates_example_type lhs, VW::slates::example_type rhs)
{
  switch (rhs)
  {
    case VW::slates::example_type::UNSET:
      return lhs == fb::CCB_Slates_example_type_unset;
    case VW::slates::example_type::ACTION:
      return lhs == fb::CCB_Slates_example_type_action;
    case VW::slates::example_type::SHARED:
      return lhs == fb::CCB_Slates_example_type_shared;
    case VW::slates::example_type::SLOT:
      return lhs == fb::CCB_Slates_example_type_slot;
    default:
      THROW("Slates label example type not currently supported");
  }
}

void prototype_label_t::verify_slates_label(const fb::Slates_Label* actual_label) const
{
  EXPECT_TRUE(are_equal(actual_label->example_type(), label.slates.type));
  EXPECT_FLOAT_EQ(actual_label->weight(), label.slates.weight);
  EXPECT_FLOAT_EQ(actual_label->cost(), label.slates.cost);
  EXPECT_EQ(actual_label->slot(), label.slates.slot_id);
  EXPECT_EQ(actual_label->labeled(), label.slates.labeled);
  EXPECT_EQ(actual_label->probabilities()->size(), label.slates.probabilities.size());

  for (size_t i = 0; i < actual_label->probabilities()->size(); i++)
  {
    auto actual_prob = actual_label->probabilities()->Get(i);
    auto expected_prob = label.slates.probabilities[i];

    EXPECT_EQ(actual_prob->action(), expected_prob.action);
    EXPECT_FLOAT_EQ(actual_prob->score(), expected_prob.score);
  }
}

void prototype_label_t::verify_slates_label(const VW::example& e) const
{
  using label_t = VW::slates::label;

  const label_t actual_label = e.l.slates;

  EXPECT_EQ(actual_label.type, label.slates.type);
  EXPECT_FLOAT_EQ(actual_label.weight, label.slates.weight);
  EXPECT_FLOAT_EQ(actual_label.cost, label.slates.cost);
  EXPECT_EQ(actual_label.slot_id, label.slates.slot_id);
  EXPECT_EQ(actual_label.labeled, label.slates.labeled);
  EXPECT_EQ(actual_label.probabilities.size(), label.slates.probabilities.size());

  for (size_t i = 0; i < actual_label.probabilities.size(); i++)
  {
    EXPECT_EQ(actual_label.probabilities[i].action, label.slates.probabilities[i].action);
    EXPECT_FLOAT_EQ(actual_label.probabilities[i].score, label.slates.probabilities[i].score);
  }
}

prototype_label_t no_label()
{
  VW::polylabel actual_label;
  actual_label.empty = {};

  return prototype_label_t{fb::Label_NONE, actual_label, {}};
}

prototype_label_t simple_label(float label, float weight, float initial)
{
  VW::reduction_features reduction_features;
  reduction_features.get<VW::simple_label_reduction_features>().weight = weight;
  reduction_features.get<VW::simple_label_reduction_features>().initial = initial;

  VW::polylabel actual_label;
  actual_label.simple.label = label;

  return prototype_label_t{fb::Label_SimpleLabel, actual_label, reduction_features};
}

prototype_label_t cb_label(std::vector<VW::cb_class> costs, float weight)
{
  VW::polylabel actual_label;
  actual_label.cb = {std::move(costs), weight};

  return prototype_label_t{fb::Label_CBLabel, actual_label, {}};
}

prototype_label_t cb_label(VW::cb_class single_class, float weight)
{
  VW::polylabel actual_label;
  actual_label.cb = {{single_class}, weight};

  return prototype_label_t{fb::Label_CBLabel, actual_label, {}};
}

prototype_label_t cb_label_shared()
{
  /*
    const auto& costs = ec.l.cb.costs;
  if (costs.size() != 1) { return false; }
  if (costs[0].probability == -1.f) { return true; }
  return false;

  */
  return cb_label(VW::cb_class(0., 0, -1.), 1.);
}

prototype_label_t continuous_label(std::vector<VW::cb_continuous::continuous_label_elm> costs)
{
  VW::polylabel actual_label;
  v_array<VW::cb_continuous::continuous_label_elm> costs_v;
  costs_v.reserve(costs.size());
  for (size_t i = 0; i < costs.size(); i++) { costs_v.push_back(costs[i]); }

  actual_label.cb_cont = {costs_v};

  return prototype_label_t{fb::Label_ContinuousLabel, actual_label, {}};
}

prototype_label_t slates_label_raw(VW::slates::example_type type, float weight, bool labeled, float cost,
    uint32_t slot_id, std::vector<VW::action_score> probabilities)
{
  VW::slates::label slates_label;
  slates_label.type = type;
  slates_label.weight = weight;
  slates_label.labeled = labeled;
  slates_label.cost = cost;
  slates_label.slot_id = slot_id;

  slates_label.probabilities.reserve(probabilities.size());
  for (const auto& action_score : probabilities) { slates_label.probabilities.push_back(action_score); }

  VW::polylabel actual_label;
  actual_label.slates = slates_label;

  return prototype_label_t{fb::Label_Slates_Label, actual_label, {}};
}

}  // namespace vwtest
