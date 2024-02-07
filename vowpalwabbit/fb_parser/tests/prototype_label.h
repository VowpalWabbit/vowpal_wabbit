// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/future_compat.h"
#include "vw/core/example.h"
#include "vw/core/reduction_features.h"
#include "vw/core/vw.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

  void verify(VW::workspace& w, const fb::Example* ex) const;
  void verify(VW::workspace& w, const VW::example& ex) const;

  void verify(VW::workspace& w, fb::Label label_type, const void* label) const;

private:
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
};

prototype_label_t no_label();

prototype_label_t simple_label(float label, float weight, float initial = 0.f);

prototype_label_t cb_label(std::vector<VW::cb_class> costs, float weight = 1.0f);
prototype_label_t cb_label(VW::cb_class single_class, float weight = 1.0f);
prototype_label_t cb_label_shared();
}  // namespace vwtest