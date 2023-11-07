// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/io/io_adapter.h"

#include "vw/common/vw_throw.h"
#include "vw/io/errno_handling.h"

#ifdef _WIN32
#  define NOMINMAX
#  define ssize_t int64_t
#  include <io.h>
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#  include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#if (ZLIB_VERNUM < 0x1252)
typedef void* gzFile;
#else
struct gzFile_s;
using gzFile = struct gzFile_s*;
#endif

#ifndef O_LARGEFILE  // for OSX
#  define O_LARGEFILE 0
#endif

using namespace VW::io;

enum class file_mode
{
  READ,
  WRITE
};

int get_stdin_fileno()
{
#ifdef _WIN32
  return _fileno(stdin);
#else
  return fileno(stdin);
#endif
}

int get_stdout_fileno()
{
#ifdef _WIN32
  return _fileno(stdout);
#else
  return fileno(stdout);
#endif
}

class socket_adapter : public writer, public reader
{
public:
  socket_adapter(int fd, const std::shared_ptr<details::socket_closer>& closer)
      : reader(false /*is_resettable*/), _socket_fd{fd}, _closer{closer}
  {
  }
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  int _socket_fd;
  std::shared_ptr<details::socket_closer> _closer;
};

class file_adapter : public writer, public reader
{
public:
  // investigate whether not using the old flags affects perf. Old claim:
  // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
  file_adapter(const char* filename, file_mode mode);
  file_adapter(int file_descriptor, file_mode mode, bool should_close);
  ~file_adapter() override;
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;

private:
  int _file_descriptor;
  file_mode _mode;
  bool _should_close;
};

class stdio_adapter : public writer, public reader
{
public:
  stdio_adapter()
      : reader(false /*is_resettable*/)
      , _stdin_file(get_stdin_fileno(), file_mode::READ, false)
      , _stdout_file(get_stdout_fileno(), file_mode::WRITE, false)
  {
  }
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  file_adapter _stdin_file;
  file_adapter _stdout_file;
};

class gzip_file_adapter : public writer, public reader
{
public:
  gzip_file_adapter(const char* filename, file_mode mode);
  gzip_file_adapter(int file_descriptor, file_mode mode);
  ~gzip_file_adapter() override;

  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;

private:
  gzFile _gz_file;
  file_mode _mode;
};

class gzip_stdio_adapter : public writer, public reader
{
public:
  gzip_stdio_adapter();
  ~gzip_stdio_adapter() override;
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  gzFile _gz_stdin;
  gzFile _gz_stdout;
};

class custom_func_writer : public writer
{
public:
  custom_func_writer(void* context, write_func_t write_func);
  ~custom_func_writer() override = default;
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  void* _context;
  write_func_t _write_func;
};

class vector_writer : public writer
{
public:
  vector_writer(std::shared_ptr<std::vector<char>>& buffer);
  ~vector_writer() override = default;
  ssize_t write(const char* buffer, size_t num_bytes) override;

private:
  std::shared_ptr<std::vector<char>> _buffer;
};

class buffer_view : public reader
{
public:
  buffer_view(const char* data, size_t len);
  ~buffer_view() override = default;
  ssize_t read(char* buffer, size_t num_bytes) override;
  void reset() override;

private:
  const char* _data;
  const char* _read_head;
  size_t _len;
};

namespace VW
{
namespace io
{

void reader::reset() { THROW("Reset not supported for this io_adapter"); }
std::unique_ptr<writer> open_file_writer(const std::string& file_path)
{
  return std::unique_ptr<writer>(new file_adapter(file_path.c_str(), file_mode::WRITE));
}

std::unique_ptr<reader> open_file_reader(const std::string& file_path)
{
  return std::unique_ptr<reader>(new file_adapter(file_path.c_str(), file_mode::READ));
}

std::unique_ptr<writer> open_compressed_file_writer(const std::string& file_path)
{
  return std::unique_ptr<writer>(new gzip_file_adapter(file_path.c_str(), file_mode::WRITE));
}

std::unique_ptr<reader> open_compressed_file_reader(const std::string& file_path)
{
  return std::unique_ptr<reader>(new gzip_file_adapter(file_path.c_str(), file_mode::READ));
}

std::unique_ptr<reader> open_compressed_stdin() { return std::unique_ptr<reader>(new gzip_stdio_adapter()); }

std::unique_ptr<writer> open_compressed_stdout() { return std::unique_ptr<writer>(new gzip_stdio_adapter()); }

std::unique_ptr<reader> open_stdin() { return std::unique_ptr<reader>(new stdio_adapter); }

std::unique_ptr<writer> open_stdout() { return std::unique_ptr<writer>(new stdio_adapter); }

std::unique_ptr<socket> wrap_socket_descriptor(int fd) { return std::unique_ptr<socket>(new socket(fd)); }

std::unique_ptr<writer> create_custom_writer(void* context, write_func_t write_func)
{
  return std::unique_ptr<writer>(new custom_func_writer(context, write_func));
}

std::unique_ptr<writer> create_vector_writer(std::shared_ptr<std::vector<char>>& buffer)
{
  return std::unique_ptr<writer>(new vector_writer(buffer));
}

std::unique_ptr<reader> create_buffer_view(const char* data, size_t len)
{
  return std::unique_ptr<reader>(new buffer_view(data, len));
}
}  // namespace io
}  // namespace VW

//
// socket_adapter
//

ssize_t socket_adapter::read(char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return recv(_socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::read(_socket_fd, buffer, static_cast<unsigned int>(num_bytes));
#endif
}

ssize_t socket_adapter::write(const char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return send(_socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::write(_socket_fd, buffer, static_cast<unsigned int>(num_bytes));
#endif
}

details::socket_closer::socket_closer(int fd) : _socket_fd(fd) {}

details::socket_closer::~socket_closer()
{
#ifdef _WIN32
  closesocket(_socket_fd);
#else
  close(_socket_fd);
#endif
}

std::unique_ptr<reader> socket::get_reader()
{
  return std::unique_ptr<reader>(new socket_adapter(_socket_fd, _closer));
}

std::unique_ptr<writer> socket::get_writer()
{
  return std::unique_ptr<writer>(new socket_adapter(_socket_fd, _closer));
}

//
// stdio_adapter
//

ssize_t stdio_adapter::read(char* buffer, size_t num_bytes) { return _stdin_file.read(buffer, num_bytes); }

ssize_t stdio_adapter::write(const char* buffer, size_t num_bytes) { return _stdout_file.write(buffer, num_bytes); }

//
// file_adapter
//

file_adapter::file_adapter(const char* filename, file_mode mode)
    : reader(true /*is_resettable*/), _mode(mode), _should_close(true)
{
#ifdef _WIN32
  if (_mode == file_mode::READ)
  {
    // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
    _sopen_s(&_file_descriptor, filename, _O_RDONLY | _O_BINARY | _O_SEQUENTIAL, _SH_DENYWR, 0);
  }
  else
  {
    _sopen_s(
        &_file_descriptor, filename, _O_CREAT | _O_WRONLY | _O_BINARY | _O_TRUNC, _SH_DENYWR, _S_IREAD | _S_IWRITE);
  }
#else
  if (_mode == file_mode::READ) { _file_descriptor = open(filename, O_RDONLY | O_LARGEFILE); }
  else { _file_descriptor = open(filename, O_CREAT | O_WRONLY | O_LARGEFILE | O_TRUNC, 0666); }
#endif

  if (_file_descriptor == -1 && *filename != '\0') { THROWERRNO("can't open: " << filename); }
}

file_adapter::file_adapter(int file_descriptor, file_mode mode, bool should_close)
    : reader(true /*is_resettable*/), _file_descriptor(file_descriptor), _mode(mode), _should_close(should_close)
{
}

ssize_t file_adapter::read(char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::READ);
#ifdef _WIN32
  return ::_read(_file_descriptor, buffer, (unsigned int)num_bytes);
#else
  return ::read(_file_descriptor, buffer, static_cast<unsigned int>(num_bytes));
#endif
}

ssize_t file_adapter::write(const char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::WRITE);
#ifdef _WIN32
  return ::_write(_file_descriptor, buffer, (unsigned int)num_bytes);
#else
  return ::write(_file_descriptor, buffer, static_cast<unsigned int>(num_bytes));
#endif
}

void file_adapter::reset()
{
#ifdef _WIN32
  ::_lseek(_file_descriptor, 0, SEEK_SET);
#else
  ::lseek(_file_descriptor, 0, SEEK_SET);
#endif
}

file_adapter::~file_adapter()
{
  if (_should_close)
  {
#ifdef _WIN32
    ::_close(_file_descriptor);
#else
    ::close(_file_descriptor);
#endif
  }
}

//
// gzip_file_adapter
//

gzip_file_adapter::gzip_file_adapter(const char* filename, file_mode mode) : reader(true /*is_resettable*/), _mode(mode)
{
  const auto* file_mode_arg = _mode == file_mode::READ ? "rb" : "wb";
  _gz_file = gzopen(filename, file_mode_arg);
  // TODO test for failure
}

gzip_file_adapter::gzip_file_adapter(int file_descriptor, file_mode mode) : reader(true /*is_resettable*/), _mode(mode)
{
  const auto* file_mode_arg = _mode == file_mode::READ ? "rb" : "wb";
  _gz_file = gzdopen(file_descriptor, file_mode_arg);
}

gzip_file_adapter::~gzip_file_adapter() { gzclose(_gz_file); }

ssize_t gzip_file_adapter::read(char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::READ);

  auto num_read = gzread(_gz_file, buffer, static_cast<unsigned int>(num_bytes));
  return (num_read > 0) ? static_cast<size_t>(num_read) : 0;
}

ssize_t gzip_file_adapter::write(const char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::WRITE);

  auto num_written = gzwrite(_gz_file, buffer, static_cast<unsigned int>(num_bytes));
  return (num_written > 0) ? static_cast<size_t>(num_written) : 0;
}

void gzip_file_adapter::reset() { gzseek(_gz_file, 0, SEEK_SET); }

//
// gzip_stdio_adapter
//

gzip_stdio_adapter::gzip_stdio_adapter() : reader(false /*is_resettable*/)
{
#ifdef _WIN32
  _gz_stdin = gzdopen(_fileno(stdin), "rb");
  _gz_stdout = gzdopen(_fileno(stdout), "wb");
#else
  _gz_stdin = gzdopen(fileno(stdin), "rb");
  _gz_stdout = gzdopen(fileno(stdout), "wb");
#endif
}

gzip_stdio_adapter::~gzip_stdio_adapter()
{
  gzclose(_gz_stdin);
  gzclose(_gz_stdout);
}

ssize_t gzip_stdio_adapter::read(char* buffer, size_t num_bytes)
{
  auto num_read = gzread(_gz_stdin, buffer, static_cast<unsigned int>(num_bytes));
  return (num_read > 0) ? static_cast<size_t>(num_read) : 0;
}

ssize_t gzip_stdio_adapter::write(const char* buffer, size_t num_bytes)
{
  auto num_written = gzwrite(_gz_stdout, buffer, static_cast<unsigned int>(num_bytes));
  return (num_written > 0) ? static_cast<size_t>(num_written) : 0;
}

//
// vector_writer
//

vector_writer::vector_writer(std::shared_ptr<std::vector<char>>& buffer) : _buffer(buffer) {}

ssize_t vector_writer::write(const char* buffer, size_t num_bytes)
{
  _buffer->reserve(_buffer->size() + num_bytes);
  _buffer->insert(std::end(*_buffer), buffer, buffer + num_bytes);
  return num_bytes;
}

//
// custom_func_writer
//

custom_func_writer::custom_func_writer(void* context, write_func_t write_func)
    : _context(context), _write_func(write_func)
{
}

ssize_t custom_func_writer::write(const char* buffer, size_t num_bytes)
{
  return _write_func(_context, buffer, num_bytes);
}

//
// buffer_view
//

buffer_view::buffer_view(const char* data, size_t len) : reader(true), _data(data), _read_head(data), _len(len) {}

ssize_t buffer_view::read(char* buffer, size_t num_bytes)
{
  num_bytes = std::min((_data + _len) - _read_head, static_cast<std::ptrdiff_t>(num_bytes));
  if (num_bytes == 0) { return 0; }

  std::memcpy(buffer, _read_head, num_bytes);
  _read_head += num_bytes;

  return num_bytes;
}
void buffer_view::reset() { _read_head = _data; }
