// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/custom_streambuf.h"
#include "vw/io/io_adapter.h"
#include "vw/io/owning_stream.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

TEST(CustomOstream, CustomOstream)
{
  auto output_func = [](void* context, const char* buffer, size_t num_bytes) -> ssize_t
  {
    std::string input(buffer, num_bytes);
    EXPECT_TRUE(context == nullptr);
    EXPECT_EQ(input, "This is the test input, 123\n");
    return 0;
  };

  auto ptr = std::unique_ptr<std::streambuf>(
      new VW::io::writer_stream_buf(VW::io::create_custom_writer(nullptr, output_func)));
  VW::io::owning_ostream stream{std::move(ptr)};

  stream << "This is the test input, " << 123 << std ::endl;
}
