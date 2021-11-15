// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"
#include "model_utils.h"
#include "io_buf.h"

BOOST_AUTO_TEST_CASE(string_save_load)
{
  std::string s = "test";
  io_buf io;
  io.add_file(VW::io::open_file_writer("test_file.model"));
  VW::model_utils::write_model_field(io, s, "", false);
  io.flush();
  io.close_files();
  std::string s2;
  io.add_file(VW::io::open_file_reader("test_file.model"));
  VW::model_utils::read_model_field(io, s2);
  io.close_files();
  BOOST_CHECK_EQUAL(s, s2);
}