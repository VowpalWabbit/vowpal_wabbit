// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "io_adapter.h"

#include <cassert>
#include <memory>
#include <sstream>
#include <streambuf>

namespace VW
{
namespace io
{
class noop_output_streambuf : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};

class writer_stream_buf : public std::stringbuf
{
public:
  writer_stream_buf(std::unique_ptr<VW::io::writer>&& writer) : _writer(std::move(writer)) {}

  virtual int sync() override
  {
    auto ret = std::stringbuf::sync();
    if (ret != 0) { return ret; }

    const auto* begin = pbase();
    const auto* end = pptr();
    assert(end >= begin);
    _writer->write(begin, end - begin);
    _writer->flush();

    // Now that it has been outputted, clear the streambuf.
    str("");

    // Signal success.
    return 0;
  }

private:
  std::unique_ptr<VW::io::writer> _writer;
};

}  // namespace io
}  // namespace VW
