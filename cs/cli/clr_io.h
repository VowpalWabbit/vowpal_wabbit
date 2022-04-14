// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/io_adapter.h"

using namespace System;
using namespace System::IO;

namespace VW
{
/// <summary>
/// C++ wrapper for managed <see cref="Stream"/>.
/// </summary>
class clr_stream_adapter : public VW::io::writer, public VW::io::reader
{
private:
  gcroot<Stream^> m_stream;
  gcroot<cli::array<unsigned char>^> m_buffer;

  void ensure_buffer_size(size_t nbytes);

public:
  clr_stream_adapter(Stream^ stream);
  ~clr_stream_adapter();
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;
  void flush() override;
};
}
