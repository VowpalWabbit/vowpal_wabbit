// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <cassert>

#include "v_array.h"
#include "hash.h"
#include "io/io_adapter.h"

#ifndef VW_NOEXCEPT
#  include "vw_exception.h"
#endif

/* The i/o buffer can be conceptualized as an array below:
**  _______________________________________________________________________________________
** |__________|__________|__________|__________|__________|__________|__________|__________|   **
** space.begin           space.head             space.end                       space.endarray **
**
** space.begin     = the beginning of the loaded values in the buffer
** space.head      = the end of the last-read point in the buffer
** space.end       = the end of the loaded values from file
** space.endarray  = the end of the allocated space for the array
**
** The values are ordered so that:
** space.begin <= space.head <= space.end <= space.endarray
**
** Initially space.begin == space.head since no values have been read.
**
** The interval [space.head, space.end] may be shifted down to space.begin
** if the requested number of bytes to be read is larger than the interval size.
** This is done to avoid reallocating arrays as much as possible.
*/

class io_buf
{
  // used to check-sum i/o files for corruption detection
  bool _verify_hash = false;
  uint32_t _hash = 0;
  static constexpr size_t INITIAL_BUFF_SIZE = 1 << 16;

  char* _buffer_begin = nullptr;
  char* _head = nullptr;
  size_t _buffer_populated_size = 0;
  size_t _buffer_capacity = 0;

  std::vector<std::unique_ptr<VW::io::reader>> input_files;
  std::vector<std::unique_ptr<VW::io::writer>> output_files;

  void realloc_buffer(size_t new_capacity)
  {
    _buffer_begin = reinterpret_cast<char*>(std::realloc(_buffer_begin, sizeof(char) * new_capacity));
    if (_buffer_begin == nullptr)
    { THROW_OR_RETURN("realloc of " << new_capacity << " failed in resize().  out of memory?"); }
    _buffer_capacity = new_capacity;
    memset(_buffer_begin + _buffer_populated_size, 0, sizeof(char) * (_buffer_capacity - _buffer_populated_size));
  }

public:
  io_buf()
  {
    realloc_buffer(INITIAL_BUFF_SIZE);
    _head = _buffer_begin;
  }
  ~io_buf() { free(_buffer_begin); }

  io_buf(io_buf& other) = delete;
  io_buf& operator=(io_buf& other) = delete;
  io_buf(io_buf&& other) = delete;
  io_buf& operator=(io_buf&& other) = delete;

  const std::vector<std::unique_ptr<VW::io::reader>>& get_input_files() const { return input_files; }
  const std::vector<std::unique_ptr<VW::io::writer>>& get_output_files() const { return output_files; }

  // file descriptor currently being used.
  size_t current = 0;

  void verify_hash(bool verify)
  {
    _verify_hash = verify;
    // reset the hash so that the io_buf can be re-used for loading
    // as it is done for Reload()
    if (!verify) { _hash = 0; }
  }

  uint32_t hash()
  {
    if (!_verify_hash) THROW("HASH WAS NOT CALCULATED");
    return _hash;
  }

  void add_file(std::unique_ptr<VW::io::reader>&& file)
  {
    assert(output_files.empty());
    input_files.push_back(std::move(file));
  }

  void add_file(std::unique_ptr<VW::io::writer>&& file)
  {
    assert(input_files.empty());
    output_files.push_back(std::move(file));
  }

  void reset_buffer()
  {
    _buffer_populated_size = 0;
    _head = _buffer_begin;
  }

  void reset_file(VW::io::reader* f)
  {
    f->reset();
    reset_buffer();
  }

  void set(char* p) { _head = p; }

  /// This function will return the number of input files AS WELL AS the number of output files. (because of legacy)
  size_t num_files() const { return input_files.size() + output_files.size(); }
  size_t num_input_files() const { return input_files.size(); }
  size_t num_output_files() const { return output_files.size(); }

  // You can definitely call read directly on the reader object. This function hasn't been changed yet to reduce churn
  // in the refactor.
  static ssize_t read_file(VW::io::reader* f, void* buf, size_t nbytes) { return f->read((char*)buf, nbytes); }

  ssize_t fill(VW::io::reader* f)
  {
    // if the loaded values have reached the allocated space
    if (_buffer_populated_size == _buffer_capacity)
    {  // reallocate to twice as much space
      size_t head_loc = unflushed_bytes_count();
      realloc_buffer(_buffer_capacity * 2);
      _head = _buffer_begin + head_loc;
    }
    // read more bytes from file up to the remaining allocated space
    ssize_t num_read = f->read(_buffer_begin + _buffer_populated_size, _buffer_capacity - _buffer_populated_size);
    if (num_read >= 0)
    {
      // if some bytes were actually loaded, update the end of loaded values
      _buffer_populated_size += num_read;
      return num_read;
    }

    return 0;
  }

  // This has different meanings in write and read mode:
  //   - Write mode: Number of bytes that have not yet been flushed to the output device
  //   - Read mode: The offset of the position that has been read up to so far.
  size_t unflushed_bytes_count() { return _head - _buffer_begin; }

  void flush()
  {
    if (!output_files.empty())
    {
      auto written_bytes = output_files[0]->write(_buffer_begin, unflushed_bytes_count());
      if (written_bytes != static_cast<ssize_t>(unflushed_bytes_count()))
      { std::cerr << "error, failed to write example\n"; }
      _head = _buffer_begin;
      output_files[0]->flush();
    }
  }

  bool close_file()
  {
    if (!input_files.empty())
    {
      input_files.pop_back();
      return true;
    }

    if (!output_files.empty())
    {
      output_files.pop_back();
      return true;
    }

    return false;
  }

  void close_files()
  {
    while (close_file()) {}
  }

  void buf_write(char*& pointer, size_t n);
  size_t buf_read(char*& pointer, size_t n);

  // if read_message is null, just read it in.  Otherwise do a comparison and barf on read_message.
  size_t bin_read_fixed(char* data, size_t len, const char* read_message)
  {
    if (len > 0)
    {
      char* p;
      // if the model is corrupt the number of bytes can be less then specified (as there isn't enought data available
      // in the file)
      len = buf_read(p, len);

      // compute hash for check-sum
      if (_verify_hash) { _hash = (uint32_t)uniform_hash(p, len, _hash); }

      if (*read_message == '\0') { memcpy(data, p, len); }
      else if (memcmp(data, p, len) != 0)
        THROW(read_message);
      return len;
    }
    return 0;
  }

  size_t bin_write_fixed(const char* data, size_t len)
  {
    if (len > 0)
    {
      char* p;
      buf_write(p, len);

      memcpy(p, data, len);

      // compute hash for check-sum
      if (_verify_hash) { _hash = (uint32_t)uniform_hash(p, len, _hash); }
    }
    return len;
  }

  bool isbinary();
  size_t readto(char*& pointer, char terminal);
  size_t copy_to(void* dst, size_t max_size);
  void replace_buffer(char* buf, size_t capacity);
  char* buffer_start() { return _buffer_begin; }  // This should be replaced with slicing.
};

inline size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = i.bin_read_fixed((char*)&obj_len, sizeof(obj_len), "");
  if (obj_len > len || ret < sizeof(uint32_t)) THROW("bad model format!");

  ret += i.bin_read_fixed(data, obj_len, read_message);

  return ret;
}

inline size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  o.bin_write_fixed((char*)&len, sizeof(len));
  o.bin_write_fixed(data, len);
  return (len + sizeof(len));
}

inline size_t bin_text_write(io_buf& io, char* data, size_t len, std::stringstream& msg, bool text)
{
  if (text)
  {
    size_t temp = io.bin_write_fixed(msg.str().c_str(), msg.str().size());
    msg.str("");
    return temp;
  }
  return bin_write(io, data, (uint32_t)len);
}

// a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  if (read) { return bin_read(io, data, len, read_message); }
  return bin_text_write(io, data, len, msg, text);
}

inline size_t bin_text_write_fixed(io_buf& io, char* data, size_t len, std::stringstream& msg, bool text)
{
  if (text)
  {
    size_t temp = io.bin_write_fixed(msg.str().c_str(), msg.str().size());
    msg.str("");
    return temp;
  }
  return io.bin_write_fixed(data, len);
}

// a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write_fixed(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  if (read) { return io.bin_read_fixed(data, len, read_message); }
  return bin_text_write_fixed(io, data, len, msg, text);
}

inline size_t bin_text_read_write_fixed_validated(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  size_t nbytes = bin_text_read_write_fixed(io, data, len, read_message, read, msg, text);
  if (read && len > 0)  // only validate bytes read/write if expected length > 0
  {
    if (nbytes == 0) { THROW("Unexpected end of file encountered."); }
  }
  return nbytes;
}

#define writeit(what, str)                                                                  \
  do                                                                                        \
  {                                                                                         \
    msg << str << " = " << what << " ";                                                     \
    bin_text_read_write_fixed(model_file, (char*)&what, sizeof(what), "", read, msg, text); \
  } while (0);

#define writeitvar(what, str, mywhat)                                                           \
  auto mywhat = (what);                                                                         \
  do                                                                                            \
  {                                                                                             \
    msg << str << " = " << mywhat << " ";                                                       \
    bin_text_read_write_fixed(model_file, (char*)&mywhat, sizeof(mywhat), "", read, msg, text); \
  } while (0);
