#pragma once

#include <stdio.h>
#include <sys/socket.h>

#ifdef WIN32
#include <winsock2.h>
#endif

#include "network.h"
#include <fstream>

struct io_adapter
{
  virtual ssize_t read(char* /*buffer*/, size_t /*num_bytes*/) = 0;
  virtual ssize_t write(const char* /*buffer*/, size_t /*num_bytes*/) = 0;
  virtual void reset() = 0;
  virtual ~io_adapter() {};
};

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
  socket_adapter(int fd) : socket_fd{fd}
  {}

  virtual ssize_t read(char* buffer, size_t num_bytes) override
  {
#ifdef _WIN32
  return recv(socket_fd, buffer static_cast<int>(num_bytes), 0);
#else
  return ::read(socket_fd, buffer, (unsigned int)num_bytes);
#endif
  }

  virtual ssize_t write(const char* buffer, size_t num_bytes) override
  {
#ifdef _WIN32
  return send(socket_fd, buffer, static_cast<int>(num_bytes), 0);
#else
  return ::write(socket_fd, buffer, (unsigned int)num_bytes);
#endif
  }

  virtual void reset() override
  {
    THROW("reset not supported");
  }

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
  virtual ssize_t read(char* buffer, size_t num_bytes) override
  {
    std::cin.read(buffer, num_bytes);
    return std::cin.gcount();
  }

  virtual ssize_t write(const char* buffer, size_t num_bytes) override
  {
    std::cout.write(buffer, num_bytes);
    // TODO is there a reliable way to do this.
    return num_bytes;
  }

  virtual void reset() override
  {
    THROW("reset not supported");
  }
};

struct file_adapter : public io_adapter
{
  // investigate whether not using the old flags affects perf. Old claim:
    // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
  file_adapter(const char* filename) : file_stream{filename} {}

  virtual ssize_t read(char* buffer, size_t num_bytes) override
  {
    file_stream.read(buffer, num_bytes);
    return file_stream.gcount();
  }

  virtual ssize_t write(const char* buffer, size_t num_bytes) override
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

  virtual ~gzip_file_adapter() override
  {
    gzclose(gz_file);
  }

  virtual ssize_t read(char* buffer, size_t num_bytes) override
  {
    assert(mode == file_mode::read);

    auto num_read = gzread(gz_file, buffer, num_bytes);
    return (num_read > 0) ? num_read : 0;
  }

  virtual ssize_t write(const char* buffer, size_t num_bytes) override
  {
    assert(mode == file_mode::write);

    auto num_written = gzwrite(gz_file, buffer, num_bytes);
    return (num_written > 0) ? num_written : 0;
  }

  virtual void reset() override
  {
    gzseek(gz_file, 0, SEEK_SET);
  }

  private:
    gzFile gz_file;
    file_mode mode;
};
