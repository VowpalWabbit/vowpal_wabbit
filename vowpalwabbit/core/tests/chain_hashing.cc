// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"
#include "vw/text_parser/parse_example_text.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(ChainHashing, BetweenFormats)
{
  VW::feature_index txt_idx;
  VW::feature_index json_idx;

  std::string text("1 |f a:b");
  std::string json_text = R"(
    {
      "_label": 1,
      "f": {
        "a": "b"
      }
    })";

  auto vw = VW::initialize(vwtest::make_args("--quiet", "--chain_hash"));
  {
    VW::multi_ex examples;
    examples.push_back(&VW::get_unused_example(vw.get()));
    auto example = examples[0];

    VW::parsers::text::read_line(*vw, example, text.c_str());
    setup_example(*vw, example);

    auto& indices = example->feature_space['f'].indices;
    txt_idx = indices[0];
    VW::finish_example(*vw, examples);
  }
  {
    auto examples = vwtest::parse_json(*vw, json_text);
    auto example = examples[0];

    auto& indices = example->feature_space['f'].indices;
    json_idx = indices[0];
    VW::finish_example(*vw, examples);
  }
  EXPECT_EQ(txt_idx, json_idx);
}