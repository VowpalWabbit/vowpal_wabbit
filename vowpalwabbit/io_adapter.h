#pragma once

#include <stdio.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>

#endif
#include <sys/types.h>

#include "network.h"
#include <fstream>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#define ssize_t int64_t
#include <io.h>
#include <sys/stat.h>
#endif




// namespace VW
// {
//   namespace io
//   {
struct io_adapter
{
  virtual size_t read(char* /*buffer*/, size_t /*num_bytes*/) = 0;
  virtual size_t write(const char* /*buffer*/, size_t /*num_bytes*/) = 0;
  virtual void reset() = 0;
  virtual ~io_adapter(){};
};
//   }
// }

enum class file_mode
{
  read,
  write
};

#ifdef WIN32
#include <winsock2.h>
#endif

struct socket_adapter : public io_adapter
{
  socket_adapter(int fd) : socket_fd{fd} {}

  virtual size_t read(char* buffer, size_t num_bytes) override
  {
#ifdef _WIN32
    return recv(socket_fd, buffer, (int)(num_bytes), 0);
#else
    return ::read(socket_fd, buffer, (unsigned int)num_bytes);
#endif
  }

  virtual size_t write(const char* buffer, size_t num_bytes) override
  {
#ifdef _WIN32
    return send(socket_fd, buffer, (int)(num_bytes), 0);
#else
    return ::write(socket_fd, buffer, (unsigned int)num_bytes);
#endif
  }

  virtual void reset() override { THROW("reset not supported"); }

  virtual ~socket_adapter() override
  {
#ifdef _WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif
  };

 private:
  int socket_fd;
};

struct stdio_adapter : public io_adapter
{
  virtual size_t read(char* buffer, size_t num_bytes) override
  {
    std::cin.read(buffer, num_bytes);
    return std::cin.gcount();
  }

  virtual size_t write(const char* buffer, size_t num_bytes) override
  {
    std::cout.write(buffer, num_bytes);
    // TODO is there a reliable way to do this.
    return num_bytes;
  }

  virtual void reset() override { THROW("reset not supported"); }
};

struct file_adapter : public io_adapter
{
  // investigate whether not using the old flags affects perf. Old claim:
  // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
  file_adapter(const char* filename) : file_stream{filename} {}
  file_adapter(const std::string& filename) : file_stream{filename} {}

  virtual size_t read(char* buffer, size_t num_bytes) override
  {
    file_stream.read(buffer, num_bytes);
    return file_stream.gcount();
  }

  virtual size_t write(const char* buffer, size_t num_bytes) override
  {
    file_stream.write(buffer, num_bytes);
    // TODO is there a reliable way to do this.
    return num_bytes;
  }

  virtual void reset() override
  {
    file_stream.clear();
    file_stream.seekg(0, std::ios::beg);
  }

 private:
  std::fstream file_stream;
};

#include "zlib.h"

#if (ZLIB_VERNUM < 0x1252)
typedef void* gzFile;
#else
struct gzFile_s;
typedef struct gzFile_s* gzFile;
#endif

struct gzip_file_adapter : public io_adapter
{
  gzip_file_adapter(const char* filename, file_mode mode)
  {
    auto file_mode_arg = mode == file_mode::read ? "rb" : "wb";
    gz_file = gzopen(filename, file_mode_arg);
    // TODO test for failure
  }

  gzip_file_adapter(int file_descriptor, file_mode mode)
  {
    auto file_mode_arg = mode == file_mode::read ? "rb" : "wb";
    gz_file = gzdopen(file_descriptor, file_mode_arg);
  }

  virtual ~gzip_file_adapter() override { gzclose(gz_file); }

  virtual size_t read(char* buffer, size_t num_bytes) override
  {
    assert(mode == file_mode::read);

    auto num_read = gzread(gz_file, buffer, (unsigned int)num_bytes);
    return (num_read > 0) ? (size_t)num_read : 0;
  }

  virtual size_t write(const char* buffer, size_t num_bytes) override
  {
    assert(mode == file_mode::write);

    auto num_written = gzwrite(gz_file, buffer, (unsigned int)num_bytes);
    return (num_written > 0) ? (size_t)num_written : 0;
  }

  virtual void reset() override { gzseek(gz_file, 0, SEEK_SET); }

 private:
  gzFile gz_file;
  file_mode mode;
};

struct vector_adapter : public io_adapter
{
  vector_adapter(const char* data, size_t len) : m_buffer{data, data + len}, m_iterator{m_buffer.begin()} {}

  vector_adapter() : m_iterator{m_buffer.begin()} {}

  virtual ~vector_adapter() override = default;

  virtual size_t read(char* buffer, size_t num_bytes) override
  {
    num_bytes = min(m_buffer.end() - m_iterator, num_bytes);

    memcpy(buffer, &*m_iterator, num_bytes);
    m_iterator += num_bytes;

    return num_bytes;
  }

  virtual size_t write(const char* buffer, size_t num_bytes) override
  {
    m_buffer.reserve(num_bytes);
    m_buffer.insert(std::end(m_buffer), (const char*)buffer, (const char*)buffer + num_bytes);
    return num_bytes;
  }

  virtual void reset() override { m_iterator = m_buffer.begin(); }

 private:
  std::vector<char> m_buffer;
  std::vector<char>::iterator m_iterator;
};
