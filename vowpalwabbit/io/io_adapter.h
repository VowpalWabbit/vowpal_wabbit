#pragma once

#include "../vw_exception.h"

#include <string>
#include <memory>
#include <vector>

// ssize_t
#ifdef _WIN32
#define ssize_t int64_t
#else
#include <unistd.h>
#endif

namespace VW
{
namespace io
{
namespace details
{
struct socket_closer
{
  socket_closer(int fd) : _socket_fd(fd) {}
  ~socket_closer();
private:
  int _socket_fd;
};
}

struct reader
{
  reader(bool is_resettable) : _is_resettable(is_resettable) {}
  virtual ~reader() = default;
  virtual ssize_t read(char* /*buffer*/, size_t /*num_bytes*/) = 0;
  // Users should check if this io_adapter is resetable before trying to reset.
  virtual void reset() { THROW("Reset not supported for this io_adapter"); }
  bool is_resettable() const { return _is_resettable; }

  reader(reader& other) = delete;
  reader& operator=(reader& other) = delete;
  reader(reader&& other) = delete;
  reader& operator=(reader&& other) = delete;

private:
  bool _is_resettable;
};

struct writer
{
  writer() = default;
  virtual ~writer() = default;
  virtual ssize_t write(const char* /*buffer*/, size_t /*num_bytes*/) = 0;
  // Flush is a noop, writers can optionally implement this.
  virtual void flush() {}

  writer(writer& other) = delete;
  writer& operator=(writer& other) = delete;
  writer(writer&& other) = delete;
  writer& operator=(writer&& other) = delete;
};

struct socket
{
  socket(int fd) : _socket_fd(fd)
  {
    _closer = std::make_shared<details::socket_closer>(fd);
  }
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
std::unique_ptr<socket> wrap_socket_descriptor(int fd);
std::unique_ptr<writer> create_vector_writer(std::vector<char>& buffer);
std::unique_ptr<reader> create_in_memory_reader(const char* data, size_t len);
}  // namespace io
}  // namespace VW
