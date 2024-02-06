// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "prototype_label.h"

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
        return VW::parsers::flatbuffer::CreateSimpleLabel(builder, label.simple.label, red_features.weight, red_features.initial).Union();
      }
      case fb::Label_CBLabel:
      {
        std::vector<flatbuffers::Offset<fb::CB_class>> action_costs;
        for (const auto& cost : label.cb.costs)
        {
          action_costs.push_back(fb::CreateCB_class(builder, cost.action, cost.cost, cost.probability, cost.partial_prediction));
        }

        Offset<Vector<Offset<fb::CB_class>>> action_costs_vector = builder.CreateVector(action_costs);

        return fb::CreateCBLabel(builder, label.cb.weight, action_costs_vector).Union();
      }
      default:
      {
        THROW("Label type not currently supported");
        return Offset<void>();
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
      // TODO: other label types
      default:
      {
        THROW("Label type not currently supported");
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
      // TODO: other label types
      default:
      {
        THROW("Label type not currently supported");
        break;
      }
    }
  }

  void prototype_label_t::verify_simple_label(const fb::Example* e) const
  {
    const fb::SimpleLabel* actual_label = e->label_as_SimpleLabel();
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

  void prototype_label_t::verify_cb_label(const fb::Example* e) const
  {
    const fb::CBLabel* actual_label = e->label_as_CBLabel();
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

prototype_label_t simple_label(float label, float weight, float initial)
{
  VW::reduction_features reduction_features;
  reduction_features.get<VW::simple_label_reduction_features>().weight = weight;
  reduction_features.get<VW::simple_label_reduction_features>().initial = initial;

  VW::polylabel actual_label;
  actual_label.simple.label = label;

  return prototype_label_t{fb::Label_SimpleLabel, actual_label, std::move(reduction_features)};
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
}

