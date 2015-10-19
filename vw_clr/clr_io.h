/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "io_buf.h"

using namespace System;
using namespace System::IO;

namespace VW
{
  class clr_io_buf : public io_buf
  {
  private:
    gcroot<Stream^> m_stream;

  public:
    clr_io_buf(Stream^stream);

    virtual int open_file(const char* name, bool stdin_off, int flag = READ);

    virtual void reset_file(int f);

    virtual ssize_t read_file(int f, void* buf, size_t nbytes);

    virtual size_t num_files();

    virtual ssize_t write_file(int file, const void* buf, size_t nbytes);

    virtual bool compressed();

    virtual void flush();

    virtual bool close_file();
  };
}
