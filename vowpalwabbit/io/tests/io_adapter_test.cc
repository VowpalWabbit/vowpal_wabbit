// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/io_adapter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstring>
#include <memory>

TEST(IoAdapter, IoAdapterVectorWriter)
{
  auto buffer = std::make_shared<std::vector<char>>();
  EXPECT_EQ(buffer.use_count(), 1);
  {
    auto vector_writer = VW::io::create_vector_writer(buffer);
    EXPECT_EQ(buffer.use_count(), 2);
    EXPECT_EQ(buffer->size(), 0);
    EXPECT_TRUE(*buffer == std::vector<char>{});
    EXPECT_EQ(vector_writer->write("test", 4), 4);
    EXPECT_TRUE(*buffer == (std::vector<char>{'t', 'e', 's', 't'}));
    EXPECT_EQ(vector_writer->write("more", 4), 4);
    EXPECT_TRUE(*buffer == (std::vector<char>{'t', 'e', 's', 't', 'm', 'o', 'r', 'e'}));
  }
  EXPECT_EQ(buffer.use_count(), 1);
  EXPECT_TRUE(*buffer == (std::vector<char>{'t', 'e', 's', 't', 'm', 'o', 'r', 'e'}));
}

TEST(IoAdapter, IoAdapterBufferView)
{
  constexpr std::array<const char, 13> buffer = {"test another"};
  auto buffer_reader = VW::io::create_buffer_view(buffer.data(), buffer.size());

  {
    char read_buffer[5];
    EXPECT_EQ(buffer_reader->read(read_buffer, 5), 5);
    EXPECT_EQ(std::strncmp(read_buffer, "test ", 5), 0);
  }

  {
    // Even though we ask to read 20 chars, we only get 8 because that's all that's left!
    char read_buffer2[20];
    EXPECT_EQ(buffer_reader->read(read_buffer2, 20), 8);
    EXPECT_EQ(std::strncmp(read_buffer2, "another\0", 8), 0);
  }

  {
    EXPECT_EQ(buffer_reader->is_resettable(), true);
    EXPECT_NO_THROW(buffer_reader->reset());
  }

  {
    char read_buffer3[13];
    EXPECT_EQ(buffer_reader->read(read_buffer3, 13), 13);
    // \0 is in the string constant
    EXPECT_EQ(std::strncmp(read_buffer3, "test another", 13), 0);
  }
}
