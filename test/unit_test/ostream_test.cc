// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <string>

#include "io/owning_stream.h"
#include "io/custom_streambuf.h"
#include "io/io_adapter.h"
#include "memory.h"

BOOST_AUTO_TEST_CASE(test_custom_ostream)
{
  std::string output_received;
  auto output_func = [](void* context, const char* buffer, size_t num_bytes) -> ssize_t {
    std::string input(buffer, num_bytes);
    auto* output = static_cast<std::string*>(context);
    *output += input;
    return 0;
  };

  VW::io::owning_ostream stream{
      VW::make_unique<VW::io::writer_stream_buf>(VW::io::create_custom_writer(&output_received, output_func))};

  stream << "This is the test input, " << 123 << std ::endl;
  stream.flush();
  BOOST_CHECK_EQUAL(output_received, "This is the test input, 123\n");
}
