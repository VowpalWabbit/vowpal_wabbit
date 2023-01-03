// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/hash.h"
#include "vw/common/string_view.h"
#include "vw/io/io_adapter.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#ifndef VW_NOEXCEPT
#  include "vw/common/vw_exception.h"
#  include "vw/common/vw_throw.h"
#endif

/* The i/o buffer can be conceptualized as an array below:
**  _________________________________________________________________
** |__________|__________|__________|__________|__________|__________|
** _buffer.begin        head       _buffer.end                     _buffer.end_array
**
** _buffer.begin     = the beginning of the loaded values in the buffer
** head               = the end of the last-read point in the buffer
** _buffer.end       = the end of the loaded values from file
** _buffer.end_array = the end of the allocated _buffer for the array
**
** The values are ordered so that:
** _buffer.begin <= head <= _buffer.end <= _buffer.end_array
**
** Initially _buffer.begin == head since no values have been read.
**
** The interval [head, _buffer.end] may be shifted down to _buffer.begin
** if the requested number of bytes to be read is larger than the interval size.
** This is done to avoid reallocating arrays as much as possible.
*/
namespace VW
{

class io_buf
{
public:
  io_buf()
  {
    _buffer.realloc(INITIAL_BUFF_SIZE);
    _head = _buffer.begin;
  }

  io_buf(io_buf& other) = delete;
  io_buf& operator=(io_buf& other) = delete;
  io_buf(io_buf&& other) = delete;
  io_buf& operator=(io_buf&& other) = delete;

  const std::vector<std::unique_ptr<VW::io::reader>>& get_input_files() const { return _input_files; }
  const std::vector<std::unique_ptr<VW::io::writer>>& get_output_files() const { return _output_files; }

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
    assert(_output_files.empty());
    _input_files.push_back(std::move(file));
  }

  void add_file(std::unique_ptr<VW::io::writer>&& file)
  {
    assert(_input_files.empty());
    _output_files.push_back(std::move(file));
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

  void set(char* p) { _head = p; }

  /// This function will return the number of input files AS WELL AS the number of output files. (because of legacy)
  size_t num_files() const { return _input_files.size() + _output_files.size(); }
  size_t num_input_files() const { return _input_files.size(); }
  size_t num_output_files() const { return _output_files.size(); }

  // You can definitely call read directly on the reader object. This function hasn't been changed yet to reduce churn
  // in the refactor.
  static ssize_t read_file(VW::io::reader* f, void* buf, size_t nbytes)
  {
    return f->read(static_cast<char*>(buf), nbytes);
  }

  ssize_t fill(VW::io::reader* f)
  {
    // if the loaded values have reached the allocated space
    if (_buffer.end_array - _buffer.end == 0)
    {  // reallocate to twice as much space
      size_t head_loc = unflushed_bytes_count();
      _buffer.realloc(_buffer.capacity() * 2);
      _head = _buffer.begin + head_loc;
    }
    // read more bytes from file up to the remaining allocated space
    ssize_t num_read = f->read(_buffer.end, _buffer.end_array - _buffer.end);
    if (num_read >= 0)
    {
      // if some bytes were actually loaded, update the end of loaded values
      _buffer.end += num_read;
      return num_read;
    }

    return 0;
  }

  // This has different meanings in write and read mode:
  //   - Write mode: Number of bytes that have not yet been flushed to the output device
  //   - Read mode: The offset of the position that has been read up to so far.
  size_t unflushed_bytes_count() { return _head - _buffer.begin; }

  void flush();

  bool close_file()
  {
    if (!_input_files.empty())
    {
      _input_files.pop_back();
      return true;
    }

    if (!_output_files.empty())
    {
      _output_files.pop_back();
      return true;
    }

    return false;
  }

  void close_files()
  {
    while (close_file()) {}
  }

  template <typename T,
      typename std::enable_if<!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value, bool>::type = true>
  size_t write_value(const T& value)
  {
    char* c;
    buf_write(c, sizeof(T));
    memcpy(c, &value, sizeof(T));
    c += sizeof(T);
    set(c);
    return sizeof(T);
  }

  template <typename T,
      typename std::enable_if<!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value, bool>::type = true>
  T read_value(VW::string_view debug_name = "")
  {
    char* read_head = nullptr;
    T value;
    if (buf_read(read_head, sizeof(T)) < sizeof(T))
    {
      if (!debug_name.empty()) { THROW("Failed to read cache value: " << debug_name << ", with size: " << sizeof(T)); }
      else { THROW("Failed to read cache value with size: " << sizeof(T)); }
    }
    memcpy(&value, read_head, sizeof(T));
    return value;
  }

  template <typename T,
      typename std::enable_if<!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value, bool>::type = true>
  T read_value_and_accumulate_size(VW::string_view debug_name, size_t& size)
  {
    size += sizeof(T);
    return read_value<T>(debug_name);
  }

  void buf_write(char*& pointer, size_t n);
  size_t buf_read(char*& pointer, size_t n);

  size_t bin_read_fixed(char* data, size_t len)
  {
    if (len > 0)
    {
      char* p;
      // if the model is corrupt the number of bytes can be less then specified (as there isn't enought data available
      // in the file)
      len = buf_read(p, len);

      // compute hash for check-sum
      if (_verify_hash) { _hash = static_cast<uint32_t>(VW::uniform_hash(p, len, _hash)); }
      memcpy(data, p, len);
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
      if (_verify_hash) { _hash = static_cast<uint32_t>(VW::uniform_hash(p, len, _hash)); }
    }
    return len;
  }

  bool isbinary();
  size_t readto(char*& pointer, char terminal);
  size_t copy_to(void* dst, size_t max_size);
  void replace_buffer(char* buf, size_t capacity);
  char* buffer_start() { return _buffer.begin; }  // This should be replaced with slicing.

private:
  // io_buf requires a grow only variant of v_array where it has access to the internals.
  // It sets the begin, end and endarray members often and does not need the complexity
  // of a generic container type, hence why a thin object is defined here.
  class internal_buffer
  {
  public:
    char* begin = nullptr;
    char* end = nullptr;
    char* end_array = nullptr;

    ~internal_buffer() { std::free(begin); }

    void realloc(size_t new_capacity)
    {
      // This specific internal buffer should only ever grow.
      assert(new_capacity >= capacity());
      const auto old_size = size();
      char* temp = reinterpret_cast<char*>(std::realloc(begin, sizeof(char) * new_capacity));
      if (temp == nullptr)
      {
        // begin still needs to be freed but the destructor will do it.
        THROW_OR_RETURN("realloc of " << new_capacity << " failed in resize().  out of memory?");
      }
      begin = temp;
      end = begin + old_size;
      end_array = begin + new_capacity;
      memset(end, 0, sizeof(char) * (end_array - end));
    }

    void shift_to_front(char* head_ptr)
    {
      assert(end >= head_ptr);
      const size_t space_left = end - head_ptr;
      // Only call memmove if we are within the bounds of the loaded buffer.
      // Also, this ensures we don't memmove when head_ptr == end_array which
      // would be undefined behavior.
      if (head_ptr >= begin && head_ptr < end) { std::memmove(begin, head_ptr, space_left); }
      end = begin + space_left;
    }

    size_t capacity() const { return end_array - begin; }
    size_t size() const { return end - begin; }
  };

  // used to check-sum i/o files for corruption detection
  bool _verify_hash = false;
  uint32_t _hash = 0;
  static constexpr size_t INITIAL_BUFF_SIZE = 1 << 16;

  internal_buffer _buffer;
  char* _head = nullptr;

  // file descriptor currently being used.
  size_t _current = 0;

  std::vector<std::unique_ptr<VW::io::reader>> _input_files;
  std::vector<std::unique_ptr<VW::io::writer>> _output_files;
};

namespace details
{
inline size_t bin_read(io_buf& i, char* data, size_t len)
{
  uint32_t obj_len;
  size_t ret = i.bin_read_fixed(reinterpret_cast<char*>(&obj_len), sizeof(obj_len));
  if (obj_len > len || ret < sizeof(uint32_t)) THROW("Bad model format.");

  ret += i.bin_read_fixed(data, obj_len);

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
inline size_t bin_text_read_write(io_buf& io, char* data, size_t len, bool read, std::stringstream& msg, bool text)
{
  if (read) { return bin_read(io, data, len); }
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
    io_buf& io, char* data, size_t len, bool read, std::stringstream& msg, bool text)
{
  if (read) { return io.bin_read_fixed(data, len); }
  return bin_text_write_fixed(io, data, len, msg, text);
}

inline size_t bin_text_read_write_fixed_validated(
    io_buf& io, char* data, size_t len, bool read, std::stringstream& msg, bool text)
{
  size_t nbytes = bin_text_read_write_fixed(io, data, len, read, msg, text);
  if (read && len > 0)  // only validate bytes read/write if expected length > 0
  {
    if (nbytes == 0) { THROW("Unexpected end of file encountered."); }
  }
  return nbytes;
}
}  // namespace details
}  // namespace VW

using io_buf VW_DEPRECATED("io_buf moved into VW namespace") = VW::io_buf;

// Model utils functions should be used instead.
#define DEPRECATED_WRITEIT(what, str)                                                                  \
  do {                                                                                                 \
    msg << str << " = " << what << " ";                                                                \
    ::VW::details::bin_text_read_write_fixed(model_file, (char*)&what, sizeof(what), read, msg, text); \
  } while (0);

// Model utils functions should be used instead.
#define DEPRECATED_WRITEITVAR(what, str, mywhat)                                                           \
  auto mywhat = (what);                                                                                    \
  do {                                                                                                     \
    msg << str << " = " << mywhat << " ";                                                                  \
    ::VW::details::bin_text_read_write_fixed(model_file, (char*)&mywhat, sizeof(mywhat), read, msg, text); \
  } while (0);
