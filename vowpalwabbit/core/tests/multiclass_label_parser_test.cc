// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/global_data.h"
#include "vw/core/multiclass.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/shared_data.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

void parse_label(VW::label_parser& lp, VW::string_view label, VW::polylabel& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  lp.default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  lp.parse_label(l, red_fts, mem, nullptr, words, null_logger);
}

TEST(MulticlassLabelParser, MulticlassLabelParser)
{
  auto lp = VW::multiclass_label_parser_global;
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1,2,3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1a", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    EXPECT_THROW(parse_label(lp, "1 2 3", *plabel), VW::vw_exception);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    parse_label(lp, "2", *plabel);
    EXPECT_TRUE(plabel->multi.label == 2);
    EXPECT_TRUE(plabel->multi.weight == 1.0);
  }
  {
    auto plabel = VW::make_unique<VW::polylabel>();
    parse_label(lp, "2 2", *plabel);
    EXPECT_TRUE(plabel->multi.label == 2);
    EXPECT_TRUE(plabel->multi.weight == 2.0);
  }
}