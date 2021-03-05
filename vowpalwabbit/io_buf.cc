// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "io_buf.h"

size_t io_buf::buf_read(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (_head + n <= (_buffer_begin + _buffer_populated_size))
  {
    pointer = _head;
    _head += n;
    return n;
  }
  else  // out of bytes, so refill.
  {
    if (_head != _buffer_begin)  // There exists room to shift.
    {
      // Out of buffer so swap to beginning.
      size_t populated_bytes_left_in_buffer = (_buffer_begin + _buffer_populated_size) - _head;
      memmove(_buffer_begin, _head, populated_bytes_left_in_buffer);
      _head = _buffer_begin;
      _buffer_populated_size = populated_bytes_left_in_buffer;
    }
    if (current < input_files.size() &&
        fill(input_files[current].get()) > 0)  // read more bytes from current file if present
      return buf_read(pointer, n);             // more bytes are read.
    else if (++current < input_files.size())
      return buf_read(pointer, n);  // No more bytes, so go to next file and try again.
    else
    {
      // no more bytes to read, return all that we have left.
      pointer = _head;
      _head = (_buffer_begin + _buffer_populated_size);
      return (_buffer_begin + _buffer_populated_size) - pointer;
    }
  }
}

bool io_buf::isbinary()
{
  if ((_buffer_begin + _buffer_populated_size) == _head)
    if (fill(input_files[current].get()) <= 0) return false;

  bool ret = (*_head == 0);
  if (ret) _head++;

  return ret;
}

size_t io_buf::readto(char*& pointer, char terminal)
{
  // Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = _head;
  while (pointer < (_buffer_begin + _buffer_populated_size) && *pointer != terminal) pointer++;
  if (pointer != (_buffer_begin + _buffer_populated_size))
  {
    size_t n = pointer - _head;
    _head = pointer + 1;
    pointer -= n;
    return n + 1;
  }
  else
  {
    if ((_buffer_begin + _buffer_populated_size) == (_buffer_begin + _buffer_capacity))
    {
      size_t populated_bytes_left_in_buffer = (_buffer_begin + _buffer_populated_size) - _head;
      memmove(_buffer_begin, _head, populated_bytes_left_in_buffer);
      _head = _buffer_begin;
      _buffer_populated_size = populated_bytes_left_in_buffer;
      pointer = (_buffer_begin + _buffer_populated_size);
    }
    if (current < input_files.size() && fill(input_files[current].get()) > 0)  // more bytes are read.
      return readto(pointer, terminal);
    else if (++current < input_files.size())  // no more bytes, so go to next file.
      return readto(pointer, terminal);
    else  // no more bytes to read, return everything we have.
    {
      size_t n = pointer - _head;
      _head = pointer;
      pointer -= n;
      return n;
    }
  }
}

void io_buf::buf_write(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes to write into.
  if (_head + n <= (_buffer_begin + _buffer_capacity))
  {
    pointer = _head;
    _head += n;
  }
  else  // Time to dump the file
  {
    if (_head != _buffer_begin)
      flush();
    else  // Array is short, so increase size.
    {
      realloc_buffer(2 * _buffer_capacity);
      _head = _buffer_begin;
    }
    buf_write(pointer, n);
  }
}

size_t io_buf::copy_to(void* dst, size_t max_size)
{
  size_t to_copy = std::min(unflushed_bytes_count(), max_size);
  memcpy(dst, _buffer_begin, to_copy);
  return to_copy;
}

void io_buf::replace_buffer(char* buff, size_t capacity)
{
  if (_buffer_begin != nullptr) { free(_buffer_begin); }

  _buffer_begin = buff;
  _buffer_capacity = capacity;
  _buffer_populated_size = capacity;
  _head = buff;
}
