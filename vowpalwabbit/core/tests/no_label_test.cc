// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/no_label.h"

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"
#include "vw/test_common/test_common.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

namespace
{
void parse_no_label_helper(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::label_parser_reuse_mem mem;
  VW::reduction_features red_features;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_features, mem, nullptr, words, null_logger);
}
}  // namespace

TEST(NoLabel, ParseEmptyLabel)
{
  auto lp = VW::no_label_parser_global;
  auto plabel = VW::make_unique<VW::polylabel>();
  // Empty label should parse successfully (0 tokens)
  parse_no_label_helper(lp, "", *plabel);
  // No assertion failure means success - no_label doesn't store anything
}

TEST(NoLabel, DefaultLabelFunction)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  // default_label should not throw
  lp.default_label(label);
  // No assertion failure means success
}

TEST(NoLabel, TestLabelReturnsFalse)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  lp.default_label(label);
  // test_label always returns false for no_label
  EXPECT_FALSE(lp.test_label(label));
}

TEST(NoLabel, GetWeightReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);
  // get_weight always returns 1.0 for no_label
  EXPECT_FLOAT_EQ(lp.get_weight(label, red_features), 1.f);
}

TEST(NoLabel, CacheLabelReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;
  lp.default_label(label);

  VW::io_buf cache;
  // cache_label returns 1 (successful)
  size_t result = lp.cache_label(label, red_features, cache, "", false);
  EXPECT_EQ(result, 1);
}

TEST(NoLabel, ReadCachedLabelReturnsOne)
{
  auto lp = VW::no_label_parser_global;
  VW::polylabel label;
  VW::reduction_features red_features;

  VW::io_buf cache;
  // read_cached_label returns 1 (successful)
  size_t result = lp.read_cached_label(label, red_features, cache);
  EXPECT_EQ(result, 1);
}

TEST(NoLabel, LabelTypeIsNoLabel)
{
  auto lp = VW::no_label_parser_global;
  EXPECT_EQ(lp.label_type, VW::label_type_t::NOLABEL);
}

TEST(NoLabel, IntegrationWithVWWorkspace)
{
  // Test no_label in context of a VW workspace with a reduction that uses no labels
  auto vw = VW::initialize(vwtest::make_args("--quiet"));
  EXPECT_NE(vw, nullptr);
}

TEST(NoLabel, OutputAndAccountExample)
{
  auto vw = VW::initialize(vwtest::make_args("--quiet"));

  auto* ex = VW::read_example(*vw, "| a b c");
  vw->predict(*ex);
  // Call output_and_account_no_label_example
  VW::details::output_and_account_no_label_example(*vw, *ex);
  VW::finish_example(*vw, *ex);
}
