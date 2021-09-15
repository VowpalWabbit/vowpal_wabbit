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
#include <cstdlib>
#include <algorithm>
#include <fmt/format.h>

#include "v_array.h"
#include "hash.h"
#include "io/io_adapter.h"
#include "vw_string_view.h"

#ifndef VW_NOEXCEPT
#  include "vw_exception.h"
#endif

/* The i/o buffer can be conceptualized as an array below:
**  _________________________________________________________________
** |__________|__________|__________|__________|__________|__________|
** _buffer._begin        head       _buffer._end                     _buffer._end_array
**
** _buffer._begin     = the beginning of the loaded values in the buffer
** head               = the end of the last-read point in the buffer
** _buffer._end       = the end of the loaded values from file
** _buffer._end_array = the end of the allocated _buffer for the array
**
** The values are ordered so that:
** _buffer._begin <= head <= _buffer._end <= _buffer._end_array
**
** Initially _buffer._begin == head since no values have been read.
**
** The interval [head, _buffer._end] may be shifted down to _buffer._begin
** if the requested number of bytes to be read is larger than the interval size.
** This is done to avoid reallocating arrays as much as possible.
*/

class io_buf
{
  // io_buf requires a grow only variant of v_array where it has access to the internals.
  // It sets the begin, end and endarray members often and does not need the complexity
  // of a generic container type, hence why a thin object is defined here.
  struct internal_buffer
  {
    char* _begin = nullptr;
    char* _end = nullptr;
    char* _end_array = nullptr;

    ~internal_buffer() { std::free(_begin); }

    void realloc(size_t new_capacity)
    {
      // This specific internal buffer should only ever grow.
      assert(new_capacity >= capacity());
      const auto old_size = size();
      char* temp = reinterpret_cast<char*>(std::realloc(_begin, sizeof(char) * new_capacity));
      if (temp == nullptr)
      {
        // _begin still needs to be freed but the destructor will do it.
        THROW_OR_RETURN("realloc of " << new_capacity << " failed in resize().  out of memory?");
      }
      _begin = temp;
      _end = _begin + old_size;
      _end_array = _begin + new_capacity;
      memset(_end, 0, sizeof(char) * (_end_array - _end));
    }

    void shift_to_front(char* head_ptr)
    {
      const size_t space_left = _end - head_ptr;
      memmove(_begin, head_ptr, space_left);
      _end = _begin + space_left;
    }

    size_t capacity() const { return _end_array - _begin; }
    size_t size() const { return _end - _begin; }
  };

  // used to check-sum i/o files for corruption detection
  bool _verify_hash = false;
  uint32_t _hash = 0;
  static constexpr size_t INITIAL_BUFF_SIZE = 1 << 16;

  internal_buffer _buffer;
  char* head = nullptr;

  // file descriptor currently being used.
  size_t _current = 0;

  std::vector<std::unique_ptr<VW::io::reader>> input_files;
  std::vector<std::unique_ptr<VW::io::writer>> output_files;

public:
  io_buf()
  {
    _buffer.realloc(INITIAL_BUFF_SIZE);
    head = _buffer._begin;
  }

  io_buf(io_buf& other) = delete;
  io_buf& operator=(io_buf& other) = delete;
  io_buf(io_buf&& other) = delete;
  io_buf& operator=(io_buf&& other) = delete;

  const std::vector<std::unique_ptr<VW::io::reader>>& get_input_files() const { return input_files; }
  const std::vector<std::unique_ptr<VW::io::writer>>& get_output_files() const { return output_files; }

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

  /**
   * @brief Calls reset on all contained input files. Moves back to the first
   * file being the current one processed and resets the buffer position. If the
   * contained readers are not resettable then this function will fail. The
   * buffer should be tested with io_buf::is_resettable prior to calling this to
   * avoid failure.
   */
  void reset();

  /**
   * @brief Test if this io_buf supports resetting.
   *
   * @return true if all contained input files can be reset
   * @return false if any contained input files cannot be reset
   */
  bool is_resettable() const;

  void set(char* p) { head = p; }

  /// This function will return the number of input files AS WELL AS the number of output files. (because of legacy)
  size_t num_files() const { return input_files.size() + output_files.size(); }
  size_t num_input_files() const { return input_files.size(); }
  size_t num_output_files() const { return output_files.size(); }

  // You can definitely call read directly on the reader object. This function hasn't been changed yet to reduce churn
  // in the refactor.
  static ssize_t read_file(VW::io::reader* f, void* buf, size_t nbytes)
  {
    return f->read(static_cast<char*>(buf), nbytes);
  }

  ssize_t fill(VW::io::reader* f)
  {
    // if the loaded values have reached the allocated space
    if (_buffer._end_array - _buffer._end == 0)
    {  // reallocate to twice as much space
      size_t head_loc = unflushed_bytes_count();
      _buffer.realloc(_buffer.capacity() * 2);
      head = _buffer._begin + head_loc;
    }
    // read more bytes from file up to the remaining allocated space
    ssize_t num_read = f->read(_buffer._end, _buffer._end_array - _buffer._end);
    if (num_read >= 0)
    {
      // if some bytes were actually loaded, update the end of loaded values
      _buffer._end += num_read;
      return num_read;
    }

    return 0;
  }

  // This has different meanings in write and read mode:
  //   - Write mode: Number of bytes that have not yet been flushed to the output device
  //   - Read mode: The offset of the position that has been read up to so far.
  size_t unflushed_bytes_count() { return head - _buffer._begin; }

  void flush();

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

  template <typename T>
  void write_value(const T& value)
  {
    char* c;
    buf_write(c, sizeof(T));
    *reinterpret_cast<T*>(c) = value;
    c += sizeof(T);
    set(c);
  }

  template <typename T>
  T read_value(const char* debug_name = nullptr)
  {
    char* c;
    T value;
    if (buf_read(c, sizeof(T)) < sizeof(T))
    {
      if (debug_name != nullptr)
      { THROW("Failed to read cache value: " << debug_name << ", with size: " << sizeof(T)); }
      else
      {
        THROW("Failed to read cache value with size: " << sizeof(T));
      }
    }
    value = *reinterpret_cast<T*>(c);
    c += sizeof(T);
    set(c);
    return value;
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
      if (_verify_hash) { _hash = static_cast<uint32_t>(uniform_hash(p, len, _hash)); }

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
      if (_verify_hash) { _hash = static_cast<uint32_t>(uniform_hash(p, len, _hash)); }
    }
    return len;
  }

  bool isbinary();
  size_t readto(char*& pointer, char terminal);
  size_t copy_to(void* dst, size_t max_size);
  void replace_buffer(char* buf, size_t capacity);
  char* buffer_start() { return _buffer._begin; }  // This should be replaced with slicing.
};

inline size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = i.bin_read_fixed(reinterpret_cast<char*>(&obj_len), sizeof(obj_len), "");
  if (obj_len > len || ret < sizeof(uint32_t)) THROW("bad model format!");

  ret += i.bin_read_fixed(data, obj_len, read_message);

  return ret;
}

inline size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  o.bin_write_fixed(reinterpret_cast<char*>(&len), sizeof(len));
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
  return bin_write(io, data, static_cast<uint32_t>(len));
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

namespace VW
{
template<typename T>
size_t process_model_field(io_buf& io, T& var, bool read, VW::string_view name, bool text)
{
  auto* data = reinterpret_cast<char*>(&var);
  auto len = sizeof(var);

  if (read) { return io.bin_read_fixed(data, len, ""); }

  if (text)
  {
    std::string msg = fmt::format("{} = {}\n", name, var);
    return io.bin_write_fixed(msg.c_str(), msg.size());
  }

  return io.bin_write_fixed(data, len);
}
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
