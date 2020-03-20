// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "io_buf.h"
#include <cstdio>
#ifdef _WIN32
#include <winsock2.h>
#endif

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
    if (fill(files[current]) > 0)   // read more bytes from current file if present
      return buf_read(pointer, n);  // more bytes are read.
    else if (++current < files.size())
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
    if (fill(files[current]) <= 0)
      return false;

  bool ret = (*head == 0);
  if (ret)
    head++;

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
    if (current < files.size() && fill(files[current]) > 0)  // more bytes are read.
      return readto(pointer, terminal);
    else if (++current < files.size())  // no more bytes, so go to next file.
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
      space.resize(2 * (space.end_array - space.begin()));
      space.end() = space.begin();
      head = space.begin();
    }
    buf_write(pointer, n);
  }
}

bool io_buf::is_socket(int f)
{
  // this appears to work in practice, but could probably be done in a cleaner fashion
#ifdef _WIN32
  const int _nhandle = _getmaxstdio() / 2;
  return f >= _nhandle;
#else
  const int _nhandle = 32;
  return f >= _nhandle;
#endif
}

ssize_t io_buf::read_file_or_socket(int f, void* buf, size_t nbytes)
{
#ifdef _WIN32
  if (is_socket(f))
    return recv(f, reinterpret_cast<char*>(buf), static_cast<int>(nbytes), 0);
  else
    return _read(f, buf, (unsigned int)nbytes);
#else
  return read(f, buf, (unsigned int)nbytes);
#endif
}

ssize_t io_buf::write_file_or_socket(int f, const void* buf, size_t nbytes)
{
#ifdef _WIN32
  if (is_socket(f))
    return send(f, reinterpret_cast<const char*>(buf), static_cast<int>(nbytes), 0);
  else
    return _write(f, buf, (unsigned int)nbytes);
#else
  return write(f, buf, (unsigned int)nbytes);
#endif
}

void io_buf::close_file_or_socket(int f)
{
#ifdef _WIN32
  if (io_buf::is_socket(f))
    closesocket(f);
  else
    _close(f);
#else
  close(f);
#endif
}

//make all data that was writen available for reading
void io_buf::flip_from_write_to_read()
{
  space.end() = head;
  head = space.begin();
}