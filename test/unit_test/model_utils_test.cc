// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "model_utils.h"
#include "io_buf.h"
#include "example.h"

BOOST_AUTO_TEST_CASE(string_save_load)
{
  std::string s = "test";
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io;
  io.add_file(VW::io::create_vector_writer(backing_vector));
  VW::model_utils::write_model_field(io, s, "", false);
  io.flush();
  io.close_files();
  std::string s2;
  io.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
  VW::model_utils::read_model_field(io, s2);
  io.close_files();
  BOOST_CHECK_EQUAL(s, s2);
}

BOOST_AUTO_TEST_CASE(simple_label_save_load)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io;

  // Write label
  io.add_file(VW::io::create_vector_writer(backing_vector));
  label_data simple_write = {5};
  VW::model_utils::write_model_field(io, simple_write, "", false);
  io.flush();
  io.close_files();

  // Read label
  label_data simple_read;
  io.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
  VW::model_utils::read_model_field(io, simple_read);
  io.close_files();

  // Check label
  BOOST_CHECK_EQUAL(simple_read.label, 5);
}

#include "multiclass.cc"
BOOST_AUTO_TEST_CASE(multiclass_label_save_load)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf io;

  // Write label
  io.add_file(VW::io::create_vector_writer(backing_vector));
  MULTICLASS::label_t multi_write = {7, 7.5};
  MULTICLASS::cache_label(multi_write, io);
  // VW::model_utils::write_model_field(io, multi_write, "", false);
  io.flush();
  io.close_files();

  // Read label
  MULTICLASS::label_t multi_read;
  io.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));
  VW::model_utils::read_model_field(io, multi_read);
  io.close_files();

  // Check label
  BOOST_CHECK_EQUAL(multi_read.label, 7);
  BOOST_CHECK_CLOSE(multi_read.weight, 7.5, FLOAT_TOL);
}