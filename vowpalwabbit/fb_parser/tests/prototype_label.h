// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/example.h"
#include "vw/core/reduction_features.h"
#include "vw/core/vw.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"

#ifndef VWFB_BUILDERS_ONLY
#  include <gmock/gmock.h>
#  include <gtest/gtest.h>
#endif

namespace fb = VW::parsers::flatbuffer;
using namespace flatbuffers;

namespace vwtest
{

struct prototype_label_t
{
  fb::Label label_type;
  VW::polylabel label;
  VW::reduction_features reduction_features;

  Offset<void> create_flatbuffer(flatbuffers::FlatBufferBuilder& builder, VW::workspace& w) const;

#ifndef VWFB_BUILDERS_ONLY
  void verify(VW::workspace& w, const fb::Example* ex) const;
  void verify(VW::workspace& w, const VW::example& ex) const;

  void verify(VW::workspace& w, fb::Label label_type, const void* label) const;
#endif

private:
#ifndef VWFB_BUILDERS_ONLY
  inline void verify_simple_label(const fb::Example* ex) const
  {
    EXPECT_EQ(ex->label_type(), fb::Label_SimpleLabel);

    const fb::SimpleLabel* actual_label = ex->label_as_SimpleLabel();
    verify_simple_label(actual_label);
  }

  void verify_simple_label(const fb::SimpleLabel* label) const;
  void verify_simple_label(const VW::example& ex) const;

  inline void verify_cb_label(const fb::Example* ex) const
  {
    EXPECT_EQ(ex->label_type(), fb::Label_CBLabel);

    const fb::CBLabel* actual_label = ex->label_as_CBLabel();
    verify_cb_label(actual_label);
  }

  void verify_cb_label(const fb::CBLabel* label) const;
  void verify_cb_label(const VW::example& ex) const;

  inline void verify_continuous_label(const fb::Example* ex) const
  {
    EXPECT_EQ(ex->label_type(), fb::Label_ContinuousLabel);

    const fb::ContinuousLabel* actual_label = ex->label_as_ContinuousLabel();
    verify_continuous_label(actual_label);
  }

  void verify_continuous_label(const fb::ContinuousLabel* label) const;
  void verify_continuous_label(const VW::example& ex) const;

  inline void verify_slates_label(const fb::Example* ex) const
  {
    EXPECT_EQ(ex->label_type(), fb::Label_Slates_Label);

    const fb::Slates_Label* actual_label = ex->label_as_Slates_Label();
    verify_slates_label(actual_label);
  }

  void verify_slates_label(const fb::Slates_Label* label) const;
  void verify_slates_label(const VW::example& ex) const;
#endif
};

prototype_label_t no_label();

prototype_label_t simple_label(float label, float weight = 1.f, float initial = 0.f);

prototype_label_t cb_label(std::vector<VW::cb_class> costs, float weight = 1.0f);
prototype_label_t cb_label(VW::cb_class single_class, float weight = 1.0f);
prototype_label_t cb_label_shared();

prototype_label_t continuous_label(std::vector<VW::cb_continuous::continuous_label_elm> costs);

prototype_label_t slates_label_raw(VW::slates::example_type type, float weight, bool labeled, float cost,
    uint32_t slot_id, std::vector<VW::action_score> probabilities);

namespace slates
{
inline prototype_label_t shared()
{
  return vwtest::slates_label_raw(VW::slates::example_type::SHARED, 0.0f, false, 0.0f, 0, {});
}
inline prototype_label_t shared(float global_reward)
{
  return vwtest::slates_label_raw(VW::slates::example_type::SHARED, 0.0f, true, global_reward, 0, {});
}
inline prototype_label_t action(uint32_t for_slot)
{
  return vwtest::slates_label_raw(VW::slates::example_type::ACTION, 0.0f, false, 0.0f, for_slot, {});
}
inline prototype_label_t slot(uint32_t slot_id, std::vector<VW::action_score> probabilities = {})
{
  return vwtest::slates_label_raw(VW::slates::example_type::SLOT, 0.0f, false, 0.0f, slot_id, probabilities);
}
};  // namespace slates
}  // namespace vwtest