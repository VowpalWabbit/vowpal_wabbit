// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/core/vw.h"
#include "vw/core/vw_fwd.h"
#include "vw/test_common/test_common.h"
#include "vw/text_parser/parse_example_text.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace ::testing;

#include <string>

TEST(Cache, WriteAndReadExample)
{
  auto workspace = VW::initialize(vwtest::make_args("--quiet"));
  VW::example src_ex;
  VW::parsers::text::read_line(*workspace, &src_ex, "3.5 |ns1 example value test |ss2 ex:0.5");

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::parsers::cache::details::cache_temp_buffer temp_buffer;
  VW::parsers::cache::write_example_to_cache(
      io_writer, &src_ex, workspace->example_parser->lbl_parser, workspace->parse_mask, temp_buffer);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::multi_ex examples;
  VW::example dest_ex;
  examples.push_back(&dest_ex);
  VW::parsers::cache::read_example_from_cache(workspace.get(), io_reader, examples);
  EXPECT_EQ(io_reader.unflushed_bytes_count(), backing_vector->size());

  EXPECT_EQ(dest_ex.indices.size(), 2);
  EXPECT_EQ(dest_ex.feature_space['n'].size(), 3);
  EXPECT_EQ(dest_ex.feature_space['s'].size(), 1);

  EXPECT_THAT(src_ex.feature_space['s'].values, Pointwise(FloatNear(1e-3f), dest_ex.feature_space['s'].values));
  EXPECT_THAT(src_ex.feature_space['s'].indices, Pointwise(Eq(), dest_ex.feature_space['s'].indices));

  EXPECT_THAT(src_ex.feature_space['n'].values, Pointwise(FloatNear(1e-3f), dest_ex.feature_space['n'].values));
  EXPECT_THAT(src_ex.feature_space['n'].indices, Pointwise(Eq(), dest_ex.feature_space['n'].indices));

  EXPECT_FLOAT_EQ(src_ex.l.simple.label, dest_ex.l.simple.label);
}

TEST(Cache, WriteAndReadLargeExample)
{
  auto workspace = VW::initialize(vwtest::make_args("--quiet"));
  VW::example src_ex;
  VW::parsers::text::read_line(*workspace, &src_ex,
      "| example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|a example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|b example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|c example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|d example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|e example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1"
      "|f example value test a b:0.3 c:0.1 d e f:0.3 g h i j k l m n o p q r s t u v w x y:5.5 z a1 b1:0.343 c1:0.1 d1 "
      "e1 f1:0.3 g1 h1 i1 j1 k1 l1 m1 n1 o1 p1 q1 r1 s1 t1 u1 v1 w1 x1 y1:5.5 z1");

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::parsers::cache::details::cache_temp_buffer temp_buffer;
  VW::parsers::cache::write_example_to_cache(
      io_writer, &src_ex, workspace->example_parser->lbl_parser, workspace->parse_mask, temp_buffer);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::multi_ex examples;
  VW::example dest_ex;
  examples.push_back(&dest_ex);
  VW::parsers::cache::read_example_from_cache(workspace.get(), io_reader, examples);

  EXPECT_EQ(src_ex.indices.size(), dest_ex.indices.size());
  for (auto idx : {' ', 'a', 'b', 'c', 'd', 'e', 'f'})
  {
    EXPECT_EQ(src_ex.feature_space[idx].size(), dest_ex.feature_space[idx].size());
    EXPECT_THAT(src_ex.feature_space[idx].values, Pointwise(FloatNear(1e-3f), dest_ex.feature_space[idx].values));
    EXPECT_THAT(src_ex.feature_space[idx].indices, Pointwise(Eq(), dest_ex.feature_space[idx].indices));
  }
}

TEST(Cache, WriteAndReadTag)
{
  VW::v_array<char> tag;
  tag.push_back('m');
  tag.push_back('y');
  tag.push_back(' ');
  tag.push_back('t');
  tag.push_back('a');
  tag.push_back('g');

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::parsers::cache::details::cache_tag(io_writer, tag);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::v_array<char> read_tag;
  VW::parsers::cache::details::read_cached_tag(io_reader, read_tag);
  EXPECT_THAT(tag, Pointwise(Eq(), read_tag));
}

TEST(Cache, WriteAndReadIndex)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::namespace_index index = 79;
  VW::parsers::cache::details::cache_index(io_writer, index);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::namespace_index read_index = 0;
  VW::parsers::cache::details::read_cached_index(io_reader, read_index);

  EXPECT_EQ(index, read_index);
}

TEST(Cache, WriteAndReadFeatures)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  uint64_t mask = (1 << 18) - 1;

  VW::features feats;
  feats.push_back(1.f, 23424542 & mask);
  feats.push_back(4.f, 1231987 & mask);
  feats.push_back(1.1f, 675 & mask);
  feats.push_back(1.34f, 1 & mask);
  feats.push_back(1.1f, 567 & mask);
  VW::parsers::cache::details::cache_features(io_writer, feats, mask);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  VW::features read_feats;
  bool sorted = false;
  VW::parsers::cache::details::read_cached_features(io_reader, read_feats, sorted);

  EXPECT_EQ(feats.size(), read_feats.size());
  for (auto it = feats.begin(), read_it = read_feats.begin(); it != feats.end(); ++it, ++read_it)
  {
    EXPECT_EQ(it.index(), read_it.index());
    EXPECT_FLOAT_EQ(it.value(), read_it.value());
  }
}
