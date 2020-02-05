#include "io_adapter.h"

#ifdef _WIN32
#define ssize_t int64_t
#include <winsock2.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <cstdio>
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

struct socket_adapter : public io_adapter
{
  socket_adapter(int fd) : io_adapter(false/* is_resettable*/), socket_fd{fd} {}
  ~socket_adapter() override;
  size_t read(char* buffer, size_t num_bytes) override;
  size_t write(const char* buffer, size_t num_bytes) override;
 private:
  int socket_fd;
};

struct stdio_adapter : public io_adapter
{
  stdio_adapter(int fd) : io_adapter(false/* is_resettable*/) {}
  size_t read(char* buffer, size_t num_bytes) override;
  size_t write(const char* buffer, size_t num_bytes) override;
};

// TODO Migrate back to old file APIS
struct file_adapter : public io_adapter
{
  // investigate whether not using the old flags affects perf. Old claim:
  // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
  file_adapter(const char* filename) :
    io_adapter(true/* is_resettable*/),
    file_stream{filename}
  {}

  file_adapter(const std::string& filename) :
    io_adapter(true/* is_resettable*/),
    file_stream{filename}
  {}

  size_t read(char* buffer, size_t num_bytes) override;
  size_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;
 private:
  std::fstream file_stream;
};

struct gzip_file_adapter : public io_adapter
{
  gzip_file_adapter(const char* filename, gzip_file_mode mode);
  gzip_file_adapter(int file_descriptor, gzip_file_mode mode);
  ~gzip_file_adapter();

  size_t read(char* buffer, size_t num_bytes) override;

  size_t write(const char* buffer, size_t num_bytes) override;

  void reset() override;
 private:
  gzFile gz_file;
  gzip_file_mode mode;
};

struct vector_adapter : public io_adapter
{
  vector_adapter(const char* data, size_t len);
  vector_adapter();
  ~vector_adapter() = default;
  size_t read(char* buffer, size_t num_bytes) override;
  size_t write(const char* buffer, size_t num_bytes) override;
  void reset() override;
 private:
  std::vector<char> m_buffer;
  std::vector<char>::iterator m_iterator;
};

namespace VW {
  namespace io {
    std::unique_ptr<io_adapter> open_file(std::string& file_path)
    {
      return std::unique_ptr<io_adapter>(new file_adapter(file_path));
    }

    std::unique_ptr<io_adapter> open_file(const char* file_path)
    {
      return std::unique_ptr<io_adapter>(new file_adapter(file_path));
    }

    std::unique_ptr<io_adapter> open_compressed_file(std::string& file_path, gzip_file_mode mode)
    {
      return std::unique_ptr<io_adapter>(new gzip_file_adapter(file_path, mode));
    }

    std::unique_ptr<io_adapter> open_compressed_file(const char* file_path, gzip_file_mode mode)
    {
      return std::unique_ptr<io_adapter>(new gzip_file_adapter(file_path, mode));
    }

    std::unique_ptr<io_adapter> open_stdio()
    {
      return std::unique_ptr<io_adapter>(new stdio_adapter());
    }

    std::unique_ptr<io_adapter> take_ownership_of_socket(int fd)
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

size_t socket_adapter::read(char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return recv(socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::read(socket_fd, buffer, (unsigned int)num_bytes);
#endif
}

size_t socket_adapter::write(const char* buffer, size_t num_bytes)
{
#ifdef _WIN32
  return send(socket_fd, buffer, (int)(num_bytes), 0);
#else
  return ::write(socket_fd, buffer, (unsigned int)num_bytes);
#endif
}

socket_adapter::~socket_adapter()
{
#ifdef _WIN32
  closesocket(socket_fd);
#else
  close(socket_fd);
#endif
};

//
// stdio_adapter
//

size_t stdio_adapter::read(char* buffer, size_t num_bytes)
{
  std::cin.read(buffer, num_bytes);
  return std::cin.gcount();
}

size_t stdio_adapter::write(const char* buffer, size_t num_bytes)
{
  std::cout.write(buffer, num_bytes);
  // TODO is there a reliable way to do this.
  return num_bytes;
}

//
// file_adapter
//

size_t file_adapter::read(char* buffer, size_t num_bytes)
{
  file_stream.read(buffer, num_bytes);
  return file_stream.gcount();
}

size_t file_adapter::write(const char* buffer, size_t num_bytes)
{
  file_stream.write(buffer, num_bytes);
  // TODO is there a reliable way to do this.
  return num_bytes;
}

void file_adapter::reset()
{
  file_stream.clear();
  file_stream.seekg(0, std::ios::beg);
}

//
// gzip_file_adapter
//

gzip_file_adapter::gzip_file_adapter(const char* filename, gzip_file_mode mode) 
  : io_adapter(trie/* is_resettable*/)
{
  auto file_mode_arg = mode == gzip_file_mode::read ? "rb" : "wb";
  gz_file = gzopen(filename, file_mode_arg);
  // TODO test for failure
}

gzip_file_adapter::gzip_file_adapter(int file_descriptor, gzip_file_mode mode)
  : io_adapter(true /* is_resettable*/)
{
  auto file_mode_arg = mode == gzip_file_mode::read ? "rb" : "wb";
  gz_file = gzdopen(file_descriptor, file_mode_arg);
}

gzip_file_adapter::~gzip_file_adapter() { gzclose(gz_file); }

size_t gzip_file_adapter::read(char* buffer, size_t num_bytes)
{
  assert(mode == gzip_file_mode::read);

  auto num_read = gzread(gz_file, buffer, (unsigned int)num_bytes);
  return (num_read > 0) ? (size_t)num_read : 0;
}

size_t gzip_file_adapter::write(const char* buffer, size_t num_bytes)
{
  assert(mode == gzip_file_mode::write);

  auto num_written = gzwrite(gz_file, buffer, (unsigned int)num_bytes);
  return (num_written > 0) ? (size_t)num_written : 0;
}

void gzip_file_adapter::reset() { gzseek(gz_file, 0, SEEK_SET); }

//
// vector_adapter
//

vector_adapter::vector_adapter(const char* data, size_t len) :
  io_adapter(true/* is_resettable*/),
  m_buffer{data, data + len},
  m_iterator{m_buffer.begin()} 
{}

vector_adapter::vector_adapter() : 
  io_adapter(true/* is_resettable*/),
  m_iterator{m_buffer.begin()}
{}

size_t vector_adapter::read(char* buffer, size_t num_bytes)
{
  num_bytes = std::min(m_buffer.end() - m_iterator, num_bytes);
  if(num_bytes == 0)
    return 0;

  memcpy_s(buffer, num_bytes, &*m_iterator, num_bytes);
  m_iterator += num_bytes;

  return num_bytes;
}

size_t vector_adapter::write(const char* buffer, size_t num_bytes)
{
  m_buffer.reserve(num_bytes);
  m_buffer.insert(std::end(m_buffer), (const char*)buffer, (const char*)buffer + num_bytes);
  return num_bytes;
}

void vector_adapter::reset() { m_iterator = m_buffer.begin(); }
