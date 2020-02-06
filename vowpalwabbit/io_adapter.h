#pragma once

#include "vw_exception.h"

#include <string>

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
    struct io_adapter
    {
      io_adapter(bool is_resettable) : _is_resettable(is_resettable) {}
      virtual ~io_adapter(){};
      virtual ssize_t read(char* /*buffer*/, size_t /*num_bytes*/) = 0;
      virtual ssize_t write(const char* /*buffer*/, size_t /*num_bytes*/) = 0;
      // Flush is a noop, io_adapters can optionally implement this.
      virtual void flush() {}

      // Users should check if this io_adapter is resetable before trying to reset.
      virtual void reset() { THROW("Reset not supported for this io_adapter"); }
      bool is_resettable() const { return _is_resettable; }

      io_adapter(io_adapter& other) = delete;
      io_adapter& operator=(io_adapter& other) = delete;
      io_adapter(io_adapter&& other) = delete;
      io_adapter& operator=(io_adapter&& other) = delete;
    private:
      bool _is_resettable;
    };

    enum class file_mode
    {
      read,
      write
    };

    std::unique_ptr<io_adapter> open_file(const std::string& file_path, file_mode mode);
    std::unique_ptr<io_adapter> open_compressed_file(const std::string& file_path, file_mode mode);
    std::unique_ptr<io_adapter> open_compressed_stdio();
    std::unique_ptr<io_adapter> open_stdio();
    std::unique_ptr<io_adapter> take_ownership_of_socket(int fd);
    std::unique_ptr<io_adapter> create_vector_buffer();
    std::unique_ptr<io_adapter> create_vector_buffer(const char* data, size_t len);
  }
}

using namespace VW::io;
