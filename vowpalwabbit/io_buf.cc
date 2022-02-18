// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "io_buf.h"
#include "io/logger.h"

size_t io_buf::buf_read(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (head + n <= _buffer._end)
  {
    pointer = head;
    head += n;
    return n;
  }
  else  // out of bytes, so refill.
  {
    if (head != _buffer._begin)  // There exists room to shift.
    {
      // Out of buffer so swap to beginning.
      _buffer.shift_to_front(head);
      head = _buffer._begin;
    }
    if (_current < input_files.size() &&
        fill(input_files[_current].get()) > 0)  // read more bytes from _current file if present
      return buf_read(pointer, n);              // more bytes are read.
    else if (++_current < input_files.size())
      return buf_read(pointer, n);  // No more bytes, so go to next file and try again.
    else
    {
      // no more bytes to read, return all that we have left.
      pointer = head;
      head = _buffer._end;
      return _buffer._end - pointer;
    }
  }
}

bool io_buf::isbinary()
{
  if (_buffer._end == head)
    if (fill(input_files[_current].get()) <= 0) return false;

  bool ret = (*head == 0);
  if (ret) head++;

  return ret;
}

size_t io_buf::readto(char*& pointer, char terminal)
{
  // Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = head;
  while (pointer < _buffer._end && *pointer != terminal) pointer++;
  if (pointer != _buffer._end)
  {
    size_t n = pointer - head;
    head = pointer + 1;
    pointer -= n;
    return n + 1;
  }
  else
  {
    if (_buffer._end == _buffer._end_array)
    {
      _buffer.shift_to_front(head);
      head = _buffer._begin;
    }
    if (_current < input_files.size() && fill(input_files[_current].get()) > 0)  // more bytes are read.
      return readto(pointer, terminal);
    else if (++_current < input_files.size())  // no more bytes, so go to next file.
      return readto(pointer, terminal);
    else  // no more bytes to read, return everything we have.
    {
      size_t n = pointer - head;
      head = pointer;
      pointer -= n;
      return n;
    }
  }
}

void io_buf::buf_write(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes to write into.
  if (head + n <= _buffer._end_array)
  {
    pointer = head;
    head += n;
  }
  else  // Time to dump the file
  {
    if (head != _buffer._begin)
      flush();
    else  // Array is short, so increase size.
    {
      _buffer.realloc(2 * _buffer.capacity());
      head = _buffer._begin;
    }
    buf_write(pointer, n);
  }
}

size_t io_buf::copy_to(void* dst, size_t max_size)
{
  size_t to_copy = std::min(unflushed_bytes_count(), max_size);
  memcpy(dst, _buffer._begin, to_copy);
  return to_copy;
}

void io_buf::replace_buffer(char* buff, size_t capacity)
{
  if (_buffer._begin != nullptr) { std::free(_buffer._begin); }

  _buffer._begin = buff;
  _buffer._end = buff + capacity;
  _buffer._end_array = buff + capacity;
  head = buff;
}

void io_buf::flush()
{
  // This operation only makes sense in write mode.
  assert(input_files.empty());

  if (!output_files.empty())
  {
    auto bytes_written = output_files[0]->write(_buffer._begin, unflushed_bytes_count());
    if (bytes_written != static_cast<ssize_t>(unflushed_bytes_count())) { THROW("Failed to write example"); }
    head = _buffer._begin;
    output_files[0]->flush();
  }
}

void io_buf::reset()
{
  // This operation is only intended for read buffers.
  assert(output_files.empty());

  for (auto& f : input_files) { f->reset(); }
  _buffer._end = _buffer._begin;
  head = _buffer._begin;
  _current = 0;
}

bool io_buf::is_resettable() const
{
  // This operation is only intended for read buffers.
  assert(output_files.empty());

  return std::all_of(input_files.begin(), input_files.end(),
      [](const std::unique_ptr<VW::io::reader>& ptr) { return ptr->is_resettable(); });
}
