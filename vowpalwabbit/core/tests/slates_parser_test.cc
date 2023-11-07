// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "gmock/gmock.h"
#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/action_score.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/core/slates_label.h"
#include "vw/io/logger.h"
#include "vw/test_common/matchers.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using namespace testing;

void parse_slates_label(VW::string_view label, VW::slates::label& l)
{
  std::vector<VW::string_view> words;
  VW::tokenize(' ', label, words);
  VW::slates::default_label(l);
  VW::reduction_features red_fts;
  VW::label_parser_reuse_mem mem;
  auto null_logger = VW::io::create_null_logger();
  VW::slates::parse_label(l, mem, words, null_logger);
}

TEST(SlatesLabel, ParseLabel)
{
  {
    VW::slates::label label;
    parse_slates_label("slates shared", label);
    EXPECT_EQ(label.type, VW::slates::example_type::SHARED);
    EXPECT_FLOAT_EQ(label.cost, 0.f);
    EXPECT_EQ(label.labeled, false);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates shared 1", label);
    EXPECT_EQ(label.type, VW::slates::example_type::SHARED);
    EXPECT_FLOAT_EQ(label.cost, 1.f);
    EXPECT_EQ(label.labeled, true);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates action 1", label);
    EXPECT_EQ(label.type, VW::slates::example_type::ACTION);
    EXPECT_EQ(label.slot_id, 1);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot", label);
    EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
    EXPECT_EQ(label.labeled, false);
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot 0:0.2", label);
    EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
    EXPECT_EQ(label.labeled, true);
    auto expected = std::vector<VW::action_score>{{0, 0.2f}};
    EXPECT_THAT(label.probabilities, Pointwise(ActionScoreEqual(), expected));
  }

  {
    VW::slates::label label;
    parse_slates_label("slates slot 0:0.5,1:0.3,2:0.2", label);
    EXPECT_EQ(label.type, VW::slates::example_type::SLOT);
    EXPECT_EQ(label.labeled, true);
    auto expected = std::vector<VW::action_score>{{0, 0.5f}, {1, 0.3f}, {2, 0.2f}};
    EXPECT_THAT(label.probabilities, Pointwise(ActionScoreEqual(), expected));
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("shared", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("slates shared 0.1 too many args", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("slates shared 0.1 too many args", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("slates action", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("slates action 1,1", label), VW::vw_exception);
  }

  {
    VW::slates::label label;
    EXPECT_THROW(parse_slates_label("slates slot 0:0,1:0.5", label), VW::vw_exception);
  }
}

TEST(SlatesLabel, CacheSharedLabel)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates shared 0.5", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  EXPECT_EQ(uncached_label.type, VW::slates::example_type::SHARED);
  EXPECT_EQ(uncached_label.labeled, true);
  EXPECT_FLOAT_EQ(uncached_label.cost, 0.5);
}

TEST(SlatesLabel, CacheActionLabel)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates action 5", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  EXPECT_EQ(uncached_label.type, VW::slates::example_type::ACTION);
  EXPECT_EQ(uncached_label.labeled, false);
  EXPECT_EQ(uncached_label.slot_id, 5);
}

TEST(SlatesLabel, CacheSlotLabel)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::slates::label label;
  parse_slates_label("slates slot 0:0.5,1:0.25,2:0.25", label);
  VW::model_utils::write_model_field(io_writer, label, "", false);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::slates::label uncached_label;
  VW::slates::default_label(uncached_label);
  VW::model_utils::read_model_field(io_reader, uncached_label);

  EXPECT_EQ(uncached_label.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(uncached_label.labeled, true);
  auto expected = std::vector<VW::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}};
  EXPECT_THAT(uncached_label.probabilities, Pointwise(ActionScoreEqual(), expected));
}

TEST(SlatesLabel, CopyLabel)
{
  VW::slates::label label;
  parse_slates_label("slates slot 0:0.5,1:0.25,2:0.25", label);

  VW::slates::label copied_to;
  VW::slates::default_label(copied_to);
  copied_to = label;
  EXPECT_EQ(copied_to.type, VW::slates::example_type::SLOT);
  EXPECT_EQ(copied_to.labeled, true);
  auto expected = std::vector<VW::action_score>{{0, 0.5}, {1, 0.25}, {2, 0.25}};
  EXPECT_THAT(copied_to.probabilities, Pointwise(ActionScoreEqual(), expected));
}
