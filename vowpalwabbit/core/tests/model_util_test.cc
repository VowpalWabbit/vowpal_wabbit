// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/io_buf.h"
#include "vw/core/model_utils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

using namespace ::testing;

TEST(ModelUtil, UniquePtrNullptr)
{
  auto backing_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buffer;
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

  std::unique_ptr<char> my_ptr = nullptr;
  EXPECT_GT(VW::model_utils::write_model_field(write_buffer, my_ptr, "test", false), 0);
  write_buffer.flush();

  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size()));
  std::unique_ptr<char> my_read_ptr = nullptr;
  EXPECT_GT(VW::model_utils::read_model_field(read_buffer, my_read_ptr), 0);

  EXPECT_EQ(my_read_ptr, nullptr);
}

TEST(ModelUtil, UniquePtrOverwritesValueWithnullptr)
{
  auto backing_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buffer;
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

  std::unique_ptr<char> my_ptr = nullptr;
  EXPECT_GT(VW::model_utils::write_model_field(write_buffer, my_ptr, "test", false), 0);
  write_buffer.flush();

  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size()));
  std::unique_ptr<char> my_read_ptr = VW::make_unique<char>('c');
  EXPECT_GT(VW::model_utils::read_model_field(read_buffer, my_read_ptr), 0);

  EXPECT_EQ(my_read_ptr, nullptr);
}

TEST(ModelUtil, UniquePtrSimpleValue)
{
  auto backing_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buffer;
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

  std::unique_ptr<char> my_ptr = VW::make_unique<char>('c');
  EXPECT_GT(VW::model_utils::write_model_field(write_buffer, my_ptr, "test", false), 0);
  write_buffer.flush();

  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size()));
  std::unique_ptr<char> my_read_ptr = nullptr;
  EXPECT_GT(VW::model_utils::read_model_field(read_buffer, my_read_ptr), 0);

  EXPECT_NE(my_read_ptr, nullptr);
  EXPECT_EQ(*my_read_ptr, 'c');
}

TEST(ModelUtil, UniquePtrString)
{
  auto backing_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buffer;
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

  std::unique_ptr<std::string> my_ptr = VW::make_unique<std::string>("my string");
  EXPECT_GT(VW::model_utils::write_model_field(write_buffer, my_ptr, "test", false), 0);
  write_buffer.flush();

  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size()));
  std::unique_ptr<std::string> my_read_ptr = nullptr;
  EXPECT_GT(VW::model_utils::read_model_field(read_buffer, my_read_ptr), 0);

  EXPECT_NE(my_read_ptr, nullptr);
  EXPECT_EQ(*my_read_ptr, "my string");
}

TEST(ModelUtil, VectorUniquePtrString)
{
  auto backing_buffer = std::make_shared<std::vector<char>>();
  VW::io_buf write_buffer;
  write_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

  std::vector<std::unique_ptr<std::string>> my_ptrs;
  my_ptrs.push_back(VW::make_unique<std::string>("my string"));
  my_ptrs.push_back(VW::make_unique<std::string>("another"));
  my_ptrs.push_back(VW::make_unique<std::string>("more"));
  EXPECT_GT(VW::model_utils::write_model_field(write_buffer, my_ptrs, "test", false), 0);
  write_buffer.flush();

  VW::io_buf read_buffer;
  read_buffer.add_file(VW::io::create_buffer_view(backing_buffer->data(), backing_buffer->size()));
  std::vector<std::unique_ptr<std::string>> my_read_ptrs;
  EXPECT_GT(VW::model_utils::read_model_field(read_buffer, my_read_ptrs), 0);

  EXPECT_EQ(my_read_ptrs.size(), 3);
  EXPECT_EQ(*(my_read_ptrs[0]), "my string");
  EXPECT_EQ(*(my_read_ptrs[1]), "another");
  EXPECT_EQ(*(my_read_ptrs[2]), "more");
}
