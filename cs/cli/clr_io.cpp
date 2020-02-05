// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#define NOMINMAX
#include <msclr\marshal_cppstd.h>
#include "clr_io.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace VW
{
  clr_stream_adapter::clr_stream_adapter(Stream^ stream) :
    io_adapter(false), m_stream(stream), m_buffer(nullptr)
  {
    if (stream == nullptr)
    {
      throw gcnew ArgumentNullException("stream");  
    }
  }

  size_t clr_stream_adapter::read(char* buffer, size_t num_bytes)
  {
    ensure_buffer_size(nbytes);
    auto readBytes = m_stream->Read(m_buffer, 0, (int)nbytes);
    Marshal::Copy(m_buffer, 0, IntPtr(buf), (int)nbytes);
    return readBytes;
  }
  size_t clr_stream_adapter::write(const char* buffer, size_t num_bytes)
  {
    ensure_buffer_size(nbytes);
    Marshal::Copy(IntPtr((void*)buf), m_buffer, 0, (int)nbytes);
    m_stream->Write(m_buffer, 0, (int)nbytes);
    return nbytes;
  }

  void clr_stream_adapter::reset()
  {
    m_stream->Seek(0, SeekOrigin::Begin);
  }

  void clr_stream_adapter::flush()
  {
    m_stream->Flush();
  }

  void clr_stream_adapter::ensure_buffer_size(size_t nbytes)
  {
    if (m_buffer != nullptr && m_buffer->Length >= nbytes)
    {
      return;
    }

    m_buffer = gcnew array<unsigned char>((int)nbytes);
  }
}
