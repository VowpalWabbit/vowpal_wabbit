// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

// ssize_t
#ifdef _WIN32
#  define ssize_t int64_t
#else
#  include <unistd.h>
#endif

namespace VW
{
namespace io
{
namespace details
{
class socket_closer
{
public:
  socket_closer(int fd);
  ~socket_closer();

private:
  int _socket_fd;
};
}  // namespace details

class reader
{
public:
  reader(bool is_resettable) : _is_resettable(is_resettable) {}
  virtual ~reader() = default;

  /// Read num_bytes into buffer from this reader
  /// \param buffer buffer to read into, must be at least num_bytes in size otherwise this is undefined behavior
  /// \param num_bytes the number of bytes to read
  /// \returns the number of bytes successfully read into buffer
  virtual ssize_t read(char* buffer, size_t num_bytes) = 0;

  /// This function will throw if the reader does not support reseting. Users
  /// should check if this io_adapter is resetable before trying to reset.
  /// \throw VW::vw_exception if reader does not support resetting.
  virtual void reset();

  /// \returns true if this reader can be reset, otherwise false
  bool is_resettable() const { return _is_resettable; }

  reader(reader& other) = delete;
  reader& operator=(reader& other) = delete;
  reader(reader&& other) = delete;
  reader& operator=(reader&& other) = delete;

private:
  bool _is_resettable;
};

class writer
{
public:
  writer() = default;
  virtual ~writer() = default;

  /// Write num_bytes of bytes from buffer into this writer
  /// \param buffer buffer to write from
  /// \param num_bytes number of bytes of buffer to write. buffer must be at least this large otherwise this is
  /// undefined behavior. \returns the number of bytes successfully written
  virtual ssize_t write(const char* buffer, size_t num_bytes) = 0;

  /// Writers may implement flush - by default is a noop
  virtual void flush() {}

  writer(writer& other) = delete;
  writer& operator=(writer& other) = delete;
  writer(writer&& other) = delete;
  writer& operator=(writer&& other) = delete;
};

class socket
{
public:
  socket(int fd) : _socket_fd(fd) { _closer = std::make_shared<details::socket_closer>(fd); }
  ~socket() = default;
  std::unique_ptr<reader> get_reader();
  std::unique_ptr<writer> get_writer();

private:
  int _socket_fd;
  std::shared_ptr<details::socket_closer> _closer;
};

std::unique_ptr<writer> open_file_writer(const std::string& file_path);
std::unique_ptr<reader> open_file_reader(const std::string& file_path);
std::unique_ptr<writer> open_compressed_file_writer(const std::string& file_path);
std::unique_ptr<reader> open_compressed_file_reader(const std::string& file_path);
std::unique_ptr<reader> open_compressed_stdin();
std::unique_ptr<writer> open_compressed_stdout();
std::unique_ptr<reader> open_stdin();
std::unique_ptr<writer> open_stdout();

using write_func_t = ssize_t (*)(void*, const char*, size_t);
std::unique_ptr<writer> create_custom_writer(void* context, write_func_t write_func);

/// \param fd the file descriptor of the socket. Will take ownership of the resource.
/// \returns socket object which allows creation of readers or writers from this socket
std::unique_ptr<socket> wrap_socket_descriptor(int fd);

/// \param buffer a shared pointer is required to ensure the buffer remains
/// alive while in use. Passing this in allows callers to retrieve the results
/// of the write operations taken on this buffer.
std::unique_ptr<writer> create_vector_writer(std::shared_ptr<std::vector<char>>& buffer);

/// Creates a view over a buffer. This does **not** take ownership of or copy
/// the buffer. Therefore it is very important the buffer itself outlives this
/// reader object.
/// \param data beginning of buffer
/// \param len length of buffer
std::unique_ptr<reader> create_buffer_view(const char* data, size_t len);

}  // namespace io
}  // namespace VW
