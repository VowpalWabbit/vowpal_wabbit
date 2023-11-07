// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "c_io_adapter.h"

#include "vw/common/vw_exception.h"
#include "vw/common/vw_throw.h"
#include "vw/experimental/types.h"

c_reader::c_reader(void* context, VWReadFunc* read)
    : VW::io::reader(false /* is_resettable */), _context(context), _read_func(read)
{
}

ssize_t c_reader::read(char* buffer, size_t num_bytes)
{
  if (_read_func == nullptr) { THROW("Read function is not set. Should not be possible.") }

  size_t bytes_read = 0;
  auto result = _read_func(_context, buffer, num_bytes, &bytes_read);
  if (result != VW_success) { return -1; }
  return bytes_read;
}

c_writer::c_writer(void* context, VWWriteFunc* write) : _context(context), _write_func(write) {}

ssize_t c_writer::write(const char* buffer, size_t num_bytes)
{
  if (_write_func == nullptr) { THROW("Write function is not set. Should not be possible.") }
  size_t bytes_written = 0;
  auto result = _write_func(_context, buffer, num_bytes, &bytes_written);
  if (result != VW_success) { return -1; }
  return bytes_written;
}
