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
clr_io_buf::clr_io_buf(Stream^ stream) : m_stream(stream), m_buffer(nullptr)
{ if (stream == nullptr)
    throw gcnew ArgumentNullException("stream");

  files.push_back(0);
}

void clr_io_buf::ensure_buffer_size(size_t nbytes)
{ if (m_buffer != nullptr && m_buffer->Length >= nbytes)
    return;

  m_buffer = gcnew array<unsigned char>((int)nbytes);
}

int clr_io_buf::open_file(const char* name, bool stdin_off, int flag)
{ return 0;
}

void clr_io_buf::reset_file(int f)
{ m_stream->Seek(0, SeekOrigin::Begin);
  head = space.begin();

  space.end() = space.begin();
}

ssize_t clr_io_buf::read_file(int f, void* buf, size_t nbytes)
{ ensure_buffer_size(nbytes);

  int readBytes = m_stream->Read(m_buffer, 0, (int)nbytes);
  Marshal::Copy(m_buffer, 0, IntPtr(buf), (int)nbytes);

  return readBytes;
}

size_t clr_io_buf::num_files()
{ return 1;
}

ssize_t clr_io_buf::write_file(int file, const void* buf, size_t nbytes)
{ ensure_buffer_size(nbytes);

  Marshal::Copy(IntPtr((void*)buf), m_buffer, 0, (int)nbytes);
  m_stream->Write(m_buffer, 0, (int)nbytes);

  return nbytes;
}

bool clr_io_buf::compressed()
{ return false;
}

void clr_io_buf::flush()
{ io_buf::flush();
  m_stream->Flush();
}

bool clr_io_buf::close_file()
{ // don't close stream on purpose. Caller of SaveModel should have control when to close.
  return true;
}
}
