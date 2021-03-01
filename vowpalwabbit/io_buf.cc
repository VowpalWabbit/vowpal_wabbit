// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "io_buf.h"

size_t io_buf::buf_read(char*& pointer, size_t n)
{
  // return a pointer to the next n bytes.  n must be smaller than the maximum size.
  if (head + n <= space.end())
  {
    pointer = head;
    head += n;
    return n;
  }
  else  // out of bytes, so refill.
  {
    if (head != space.begin())  // There exists room to shift.
    {
      // Out of buffer so swap to beginning.
      size_t left = space.end() - head;
      memmove(space.begin(), head, left);
      head = space.begin();
      space.end() = space.begin() + left;
    }
    if (current < input_files.size() &&
        fill(input_files[current].get()) > 0)  // read more bytes from current file if present
      return buf_read(pointer, n);             // more bytes are read.
    else if (++current < input_files.size())
      return buf_read(pointer, n);  // No more bytes, so go to next file and try again.
    else
    {
      // no more bytes to read, return all that we have left.
      pointer = head;
      head = space.end();
      return space.end() - pointer;
    }
  }
}

bool io_buf::isbinary()
{
  if (space.end() == head)
    if (fill(input_files[current].get()) <= 0) return false;

  bool ret = (*head == 0);
  if (ret) head++;

  return ret;
}

size_t io_buf::readto(char*& pointer, char terminal)
{
  // Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = head;
  while (pointer < space.end() && *pointer != terminal) pointer++;
  if (pointer != space.end())
  {
    size_t n = pointer - head;
    head = pointer + 1;
    pointer -= n;
    return n + 1;
  }
  else
  {
    if (space.end() == space.end_array)
    {
      size_t left = space.end() - head;
      memmove(space.begin(), head, left);
      head = space.begin();
      space.end() = space.begin() + left;
      pointer = space.end();
    }
    if (current < input_files.size() && fill(input_files[current].get()) > 0)  // more bytes are read.
      return readto(pointer, terminal);
    else if (++current < input_files.size())  // no more bytes, so go to next file.
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
  if (head + n <= space.end_array)
  {
    pointer = head;
    head += n;
  }
  else  // Time to dump the file
  {
    if (head != space.begin())
      flush();
    else  // Array is short, so increase size.
    {
      space.resize(2 * space.capacity());
      space.end() = space.begin();
      head = space.begin();
    }
    buf_write(pointer, n);
  }
}

size_t io_buf::copy_to(void* dst, size_t max_size)
{
  size_t to_copy = std::min(unflushed_bytes_count(), max_size);
  memcpy(dst, space.begin(), to_copy);
  return to_copy;
}

void io_buf::replace_buffer(char* buff, size_t capacity)
{
  // TODO the following should be moved to v_array
  space.delete_v();
  space.begin() = buff;
  space.end_array = space.end() = buff + capacity;
  head = buff;
}
