#include "io_adapter.h"

#ifdef _WIN32
#define NOMINMAX
#define ssize_t int64_t
#include <winsock2.h>
#include <io.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <zlib.h>
#if (ZLIB_VERNUM < 0x1252)
using void* gzFile;
#else
struct gzFile_s;
typedef struct gzFile_s* gzFile;
#endif

#ifndef O_LARGEFILE  // for OSX
#define O_LARGEFILE 0
#endif

using namespace VW::io;

struct socket_adapter : public io_adapter
{
  socket_adapter(int fd) : io_adapter(false /*is_resettable*/), _socket_fd{fd} {}
  ~socket_adapter() override;
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
 private:
  int _socket_fd;
};

struct stdio_adapter : public io_adapter
{
  stdio_adapter() : io_adapter(false /*is_resettable*/) {}
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
};

// TODO Migrate back to old file APIS
struct file_adapter : public io_adapter
{
  // investigate whether not using the old flags affects perf. Old claim:
  // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
  file_adapter(const char* filename, file_mode mode);
  file_adapter(int file_descriptor, file_mode mode);
  ~file_adapter();
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;
 private:
  int _file_descriptor;
  file_mode _mode;
};

struct gzip_file_adapter : public io_adapter
{
  gzip_file_adapter(const char* filename, file_mode mode);
  gzip_file_adapter(int file_descriptor, file_mode mode);
  ~gzip_file_adapter();

  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;
 private:
  gzFile _gz_file;
  file_mode _mode;
};

struct gzip_stdio_adapter : public io_adapter
{
  gzip_stdio_adapter();
  ~gzip_stdio_adapter();
  ssize_t read(char* buffer, size_t num_bytes) override;
  ssize_t write(const char* buffer, size_t num_bytes) override;

 private:
  gzFile _gz_stdin;
  gzFile _gz_stdout;
};

namespace VW {
  namespace io {
    std::unique_ptr<io_adapter> open_file(const std::string& file_path, file_mode mode)
    {
      return std::unique_ptr<io_adapter>(new file_adapter(file_path.c_str(), mode));
    }

    std::unique_ptr<io_adapter> open_compressed_file(const std::string& file_path, file_mode mode)
    {
      return std::unique_ptr<io_adapter>(new gzip_file_adapter(file_path.c_str(), mode));
    }

    std::unique_ptr<io_adapter> open_compressed_stdio()
    {
      return std::unique_ptr<io_adapter>(new gzip_stdio_adapter());
    }

    std::unique_ptr<io_adapter> open_stdio()
    {
      return std::unique_ptr<io_adapter>(new stdio_adapter);
    }

    std::unique_ptr<io_adapter> wrap_socket_descriptor(int fd)
    {
      return std::unique_ptr<io_adapter>(new socket_adapter(fd));
    }

    std::unique_ptr<io_adapter> create_vector_buffer()
    {
      return std::unique_ptr<io_adapter>(new vector_adapter());
    }

    std::unique_ptr<io_adapter> create_vector_buffer(const char* data, size_t len)
    {
      return std::unique_ptr<io_adapter>(new vector_adapter(data, len));
    }
  }
}

//
// socket_adapter
//

ssize_t socket_adapter::read(char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return recv(_socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::read(_socket_fd, buffer, (unsigned int)num_bytes);
#endif
}

ssize_t socket_adapter::write(const char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return send(_socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::write(_socket_fd, buffer, (unsigned int)num_bytes);
#endif
}

socket_adapter::~socket_adapter()
{
#ifdef _WIN32
  closesocket(_socket_fd);
#else
  close(_socket_fd);
#endif
}

//
// stdio_adapter
//

ssize_t stdio_adapter::read(char* buffer, size_t num_bytes)
{
  std::cin.read(buffer, num_bytes);
  return std::cin.gcount();
}

ssize_t stdio_adapter::write(const char* buffer, size_t num_bytes)
{
  std::cout.write(buffer, num_bytes);
  // TODO is there a reliable way to do this?
  return num_bytes;
}

//
// file_adapter
//

file_adapter::file_adapter(const char* filename, file_mode mode) :
  io_adapter(true /*is_resettable*/), _mode(mode)
{
#ifdef _WIN32
  if(_mode == file_mode::read)
  {
    // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
    _sopen_s(&_file_descriptor, filename, _O_RDONLY | _O_BINARY | _O_SEQUENTIAL, _SH_DENYWR, 0);
  }
  else
  {
    _sopen_s(&_file_descriptor, filename, _O_CREAT | _O_WRONLY | _O_BINARY | _O_TRUNC, _SH_DENYWR, _S_IREAD | _S_IWRITE);
  }
#else
   if(_mode == file_mode::read)
  {
    _file_descriptor = open(filename, O_RDONLY | O_LARGEFILE);
  }
  else
  {
    _file_descriptor = open(filename, O_CREAT | O_WRONLY | O_LARGEFILE | O_TRUNC, 0666);
  }
#endif

  if (_file_descriptor == -1 && *filename != '\0')
  {
    THROWERRNO("can't open: " << filename);
  }
}

file_adapter::file_adapter(int file_descriptor, file_mode mode)
    : io_adapter(true /*is_resettable*/), _file_descriptor(file_descriptor), _mode(mode)
{
}

ssize_t file_adapter::read(char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::read);
#ifdef _WIN32
  return ::_read(_file_descriptor, buffer, (unsigned int)num_bytes);
#else
  return ::read(_file_descriptor, buffer, (unsigned int)num_bytes);
#endif
}

ssize_t file_adapter::write(const char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::write);
#ifdef _WIN32
  return ::_write(_file_descriptor, buffer, (unsigned int)num_bytes);
#else
  return ::write(_file_descriptor, buffer, (unsigned int)num_bytes);
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
#ifdef _WIN32
  ::_close(_file_descriptor);
#else
  ::close(_file_descriptor);
#endif
}

//
// gzip_file_adapter
//

gzip_file_adapter::gzip_file_adapter(const char* filename, file_mode mode)
    : io_adapter(true /*is_resettable*/), _mode(mode)
{
  auto file_mode_arg = _mode == file_mode::read ? "rb" : "wb";
  _gz_file = gzopen(filename, file_mode_arg);
  // TODO test for failure
}

gzip_file_adapter::gzip_file_adapter(int file_descriptor, file_mode mode)
  : io_adapter(true /*is_resettable*/), _mode(mode)
{
  auto file_mode_arg = _mode == file_mode::read ? "rb" : "wb";
  _gz_file = gzdopen(file_descriptor, file_mode_arg);
}

gzip_file_adapter::~gzip_file_adapter() { gzclose(_gz_file); }

ssize_t gzip_file_adapter::read(char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::read);

  auto num_read = gzread(_gz_file, buffer, (unsigned int)num_bytes);
  return (num_read > 0) ? (size_t)num_read : 0;
}

ssize_t gzip_file_adapter::write(const char* buffer, size_t num_bytes)
{
  assert(_mode == file_mode::write);

  auto num_written = gzwrite(_gz_file, buffer, (unsigned int)num_bytes);
  return (num_written > 0) ? (size_t)num_written : 0;
}

void gzip_file_adapter::reset() { gzseek(_gz_file, 0, SEEK_SET); }


//
// gzip_stdio_adapter
//

gzip_stdio_adapter::gzip_stdio_adapter() : io_adapter(false /*is_resettable*/)
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
  auto num_read = gzread(_gz_stdin, buffer, (unsigned int)num_bytes);
  return (num_read > 0) ? (size_t)num_read : 0;
}

ssize_t gzip_stdio_adapter::write(const char* buffer, size_t num_bytes)
{
  auto num_written = gzwrite(_gz_stdout, buffer, (unsigned int)num_bytes);
  return (num_written > 0) ? (size_t)num_written : 0;
}

//
// vector_adapter
//

vector_adapter::vector_adapter(const char* data, size_t len)
    : io_adapter(true /*is_resettable*/), _buffer{data, data + len}, _iterator{_buffer.begin()}
{}

vector_adapter::vector_adapter() : io_adapter(true /*is_resettable*/), _iterator{_buffer.begin()} {}

ssize_t vector_adapter::read(char* buffer, size_t num_bytes)
{
  num_bytes = std::min(_buffer.end() - _iterator, static_cast<std::ptrdiff_t>(num_bytes));
  if(num_bytes == 0)
    return 0;

  std::memcpy(buffer, &*_iterator, num_bytes);
  _iterator += num_bytes;

  return num_bytes;
}

ssize_t vector_adapter::write(const char* buffer, size_t num_bytes)
{
  _buffer.reserve(num_bytes);
  _buffer.insert(std::end(_buffer), (const char*)buffer, (const char*)buffer + num_bytes);
  return num_bytes;
}

void vector_adapter::reset() { _iterator = _buffer.begin(); }

const std::vector<char>& vector_adapter::data() const { return _buffer; }
