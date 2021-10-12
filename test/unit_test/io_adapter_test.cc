// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <memory>
#include <array>

#include "io/io_adapter.h"

BOOST_AUTO_TEST_CASE(io_adapter_vector_writer)
{
  auto buffer = std::make_shared<std::vector<char>>();
  BOOST_CHECK_EQUAL(buffer.use_count(), 1);
  {
    auto vector_writer = VW::io::create_vector_writer(buffer);
    BOOST_CHECK_EQUAL(buffer.use_count(), 2);
    BOOST_CHECK_EQUAL(buffer->size(), 0);
    BOOST_CHECK(*buffer == std::vector<char>{});
    BOOST_CHECK_EQUAL(vector_writer->write("test", 4), 4);
    BOOST_CHECK(*buffer == (std::vector<char>{'t', 'e', 's', 't'}));
    BOOST_CHECK_EQUAL(vector_writer->write("more", 4), 4);
    BOOST_CHECK(*buffer == (std::vector<char>{'t', 'e', 's', 't', 'm', 'o', 'r', 'e'}));
  }
  BOOST_CHECK_EQUAL(buffer.use_count(), 1);
  BOOST_CHECK(*buffer == (std::vector<char>{'t', 'e', 's', 't', 'm', 'o', 'r', 'e'}));
}

BOOST_AUTO_TEST_CASE(io_adapter_buffer_view)
{
  constexpr std::array<const char, 13> buffer = {"test another"};
  auto buffer_reader = VW::io::create_buffer_view(buffer.data(), buffer.size());

  {
    char read_buffer[5];
    BOOST_CHECK_EQUAL(buffer_reader->read(read_buffer, 5), 5);
    BOOST_CHECK_EQUAL(std::strncmp(read_buffer, "test ", 5), 0);
  }

  {
    // Even though we ask to read 20 chars, we only get 8 because that's all that's left!
    char read_buffer2[20];
    BOOST_CHECK_EQUAL(buffer_reader->read(read_buffer2, 20), 8);
    BOOST_CHECK_EQUAL(std::strncmp(read_buffer2, "another\0", 8), 0);
  }

  {
    BOOST_CHECK_EQUAL(buffer_reader->is_resettable(), true);
    BOOST_CHECK_NO_THROW(buffer_reader->reset());
  }

  {
    char read_buffer3[13];
    BOOST_CHECK_EQUAL(buffer_reader->read(read_buffer3, 13), 13);
    // \0 is in the string constant
    BOOST_CHECK_EQUAL(std::strncmp(read_buffer3, "test another", 13), 0);
  }
}
