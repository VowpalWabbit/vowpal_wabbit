#include "vector_io_buf.h"
#include <iostream>
#include <algorithm>

vector_io_buf::vector_io_buf(const char* data, size_t len) : _buffer(data, data + len)
{
  files.push_back(0);
  _iterator = _buffer.begin();
}

vector_io_buf::vector_io_buf() { files.push_back(0); }

int vector_io_buf::open_file(const char* name, bool stdin_off, int flag) { return 0; }

void vector_io_buf::reset_file(int f)
{
  _iterator = _buffer.begin();

  head = space.begin();
  space.end() = space.begin();
}

ssize_t vector_io_buf::read_file(int f, void* buf, size_t nbytes)
{  // make sure we don't go past the end
  nbytes = std::min(static_cast<size_t>(_buffer.end() - _iterator), nbytes);

  memcpy(buf, &*_iterator, nbytes);
  _iterator += nbytes;

  return nbytes;
}

size_t vector_io_buf::num_files() { return 1; }

ssize_t vector_io_buf::write_file(int file, const void* buf, size_t nbytes)
{
  _buffer.reserve(nbytes);
  _buffer.insert(std::end(_buffer), (const char*)buf, (const char*)buf + nbytes);

  return nbytes;
}

bool vector_io_buf::compressed() { return false; }

bool vector_io_buf::close_file() { return true; }
