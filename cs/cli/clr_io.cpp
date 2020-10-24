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
    reader(false), m_stream(stream), m_buffer(nullptr)
  {
    if (stream == nullptr)
    {
      throw gcnew ArgumentNullException("stream");  
    }
  }

  clr_stream_adapter::~clr_stream_adapter()
  {
    delete m_stream;
  }

  ssize_t clr_stream_adapter::read(char* buffer, size_t num_bytes)
  {
    ensure_buffer_size(num_bytes);
    auto readBytes = m_stream->Read(m_buffer, 0, (int)num_bytes);
    Marshal::Copy(m_buffer, 0, IntPtr(buffer), (int)num_bytes);
    return static_cast<ssize_t>(readBytes);
  }
  ssize_t clr_stream_adapter::write(const char* buffer, size_t num_bytes)
  {
    ensure_buffer_size(num_bytes);
    Marshal::Copy(IntPtr((void*)buffer), m_buffer, 0, (int)num_bytes);
    m_stream->Write(m_buffer, 0, (int)num_bytes);
    return static_cast<ssize_t>(num_bytes);
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
