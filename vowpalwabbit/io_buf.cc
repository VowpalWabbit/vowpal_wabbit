// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#include "io_buf.h"
#include <cstdio>
#ifdef WIN32
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

bool isbinary(io_buf& i)
{
  if (i.space.end() == i.head)
    if (i.fill(i.files[i.current]) <= 0)
      return false;

  bool ret = (*i.head == 0);
  if (ret)
    i.head++;

  return ret;
}

size_t readto(io_buf& i, char*& pointer, char terminal)
{
  // Return a pointer to the bytes before the terminal.  Must be less than the buffer size.
  pointer = i.head;
  while (pointer < i.space.end() && *pointer != terminal) pointer++;
  if (pointer != i.space.end())
  {
    size_t n = pointer - i.head;
    i.head = pointer + 1;
    pointer -= n;
    return n + 1;
  }
  else
  {
    if (i.space.end() == i.space.end_array)
    {
      size_t left = i.space.end() - i.head;
      memmove(i.space.begin(), i.head, left);
      i.head = i.space.begin();
      i.space.end() = i.space.begin() + left;
      pointer = i.space.end();
    }
    if (i.current < i.files.size() && i.fill(i.files[i.current]) > 0)  // more bytes are read.
      return readto(i, pointer, terminal);
    else if (++i.current < i.files.size())  // no more bytes, so go to next file.
      return readto(i, pointer, terminal);
    else  // no more bytes to read, return everything we have.
    {
      size_t n = pointer - i.head;
      i.head = pointer;
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
