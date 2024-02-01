// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "vw/core/io_buf.h"

#if false // AUDIT IO BUFFER ALIGNMENTS
#define __AUDIT_VW_IO_BUF(operation, target_align)           \
  std::cerr.flush();                                         \
  std::cerr << std::endl                                     \
            << "+++ io_buf " #operation ": @"                \
              << std::hex << reinterpret_cast<size_t>(_head) << std::dec                 \
              << " % " << target_align.align << " = " << reinterpret_cast<size_t>(_head) % target_align.align \
              << " vs " << target_align.offset << ")"  \
              << std::endl;
#else
#define __AUDIT_VW_IO_BUF(operation, target_align)
#endif


size_t VW::io_buf::buf_read(char*& pointer, size_t n, desired_align align)
{
  // return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (_head + n <= _buffer.end)
  {
    if (!align.is_aligned(_head))
    {
      if (_head > _buffer.begin + align.offset)
      {
        __AUDIT_VW_IO_BUF(SHIFT, align);
        _buffer.shift_to_front(_head, align);
      }
      else
      {
        THROW("io_buf cannot be aligned to desired alignment")
      }
    }

    __AUDIT_VW_IO_BUF(READ, align);
    pointer = _head;
    _head += n;
    return n;
  }
  else  // out of bytes, so refill.
  {
    if (_head != _buffer.begin)  // There exists room to shift.
    {
      __AUDIT_VW_IO_BUF(SHIFT, align);
      // Out of buffer so swap to beginning.
      _buffer.shift_to_front(_head, align);
    }
    if (_current < _input_files.size() && fill(_input_files[_current].get()) > 0)
    {
      __AUDIT_VW_IO_BUF(FILL, align);
                                           // read more bytes from _current file if present
      return buf_read(pointer, n, align);  // more bytes are read.
    }
    else if (++_current < _input_files.size())
    {
      __AUDIT_VW_IO_BUF(NEXT_FILE, align);
      return buf_read(pointer, n, align);  // No more bytes, so go to next file and try again.
    }
    else
    {
      // we aleady attempted to shift in this fork, so no point; we if we cannot be 
      // aligned properly, we should throw an error.
      if (!align.is_aligned(_head)) { THROW("io_buf cannot be aligned to desired alignment"); }

      __AUDIT_VW_IO_BUF(FINAL_READ, align);
      // no more bytes to read, return all that we have left.
      pointer = _head;
      _head = _buffer.end;
      return _buffer.end - pointer;
    }
  }
}

bool VW::io_buf::isbinary()
{
  if (_buffer.end == _head)
  {
    if (fill(_input_files[_current].get()) <= 0) { return false; }
  }

  bool ret = (*_head == 0);
  if (ret) { _head++; }

  return ret;
}

size_t VW::io_buf::readto(char*& pointer, char terminal) // note that "readto" assumes we are operating in byte mode,
                                                         // and thus does not support desired_alignment APIs
{
  // Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = _head;
  while (pointer < _buffer.end && *pointer != terminal) { pointer++; }
  if (pointer != _buffer.end)
  {
    size_t n = pointer - _head;
    _head = pointer + 1;
    pointer -= n;
    return n + 1;
  }
  else  // Else means we didn't find 'terminal' in the available buffer.
  {
    // Shift down if there is space at the beginning.
    if (_head != _buffer.begin)
    {
      _buffer.shift_to_front(_head);
      _head = _buffer.begin;
    }

    if (_current < _input_files.size() && fill(_input_files[_current].get()) > 0)
    {  // more bytes are read.
      return readto(pointer, terminal);
    }
    else if (++_current < _input_files.size())
    {  // no more bytes, so go to next file.
      return readto(pointer, terminal);
    }
    else  // no more bytes to read, return everything we have.
    {
      pointer = _head;
      _head = _buffer.end;
      return _buffer.end - pointer;
    }
  }
}

void VW::io_buf::buf_write(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes to write into.
  if (_head + n <= _buffer.end_array)
  {
    pointer = _head;
    _head += n;
  }
  else  // Time to dump the file
  {
    if (_head != _buffer.begin) { flush(); }
    else  // Array is short, so increase size.
    {
      _buffer.realloc(2 * _buffer.capacity());
      _head = _buffer.begin;
    }
    buf_write(pointer, n);
  }
}

size_t VW::io_buf::copy_to(void* dst, size_t max_size)
{
  size_t to_copy = std::min(unflushed_bytes_count(), max_size);
  memcpy(dst, _buffer.begin, to_copy);
  return to_copy;
}

void VW::io_buf::replace_buffer(char* buff, size_t capacity)
{
  if (_buffer.begin != nullptr) { std::free(_buffer.begin); }

  _buffer.begin = buff;
  _buffer.end = buff + capacity;
  _buffer.end_array = buff + capacity;
  _head = buff;
}

void VW::io_buf::flush()
{
  // This operation only makes sense in write mode.
  assert(_input_files.empty());

  if (!_output_files.empty())
  {
    auto bytes_written = _output_files[0]->write(_buffer.begin, unflushed_bytes_count());
    if (bytes_written != static_cast<ssize_t>(unflushed_bytes_count())) { THROW("Failed to write example"); }
    _head = _buffer.begin;
    _output_files[0]->flush();
  }
}

void VW::io_buf::reset()
{
  // This operation is only intended for read buffers.
  assert(_output_files.empty());

  for (auto& f : _input_files) { f->reset(); }
  _buffer.end = _buffer.begin;
  _head = _buffer.begin;
  _current = 0;
}

bool VW::io_buf::is_resettable() const
{
  // This operation is only intended for read buffers.
  assert(_output_files.empty());

  return std::all_of(_input_files.begin(), _input_files.end(),
      [](const std::unique_ptr<VW::io::reader>& ptr) { return ptr->is_resettable(); });
}
