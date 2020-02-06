#pragma once

#include "vw_exception.h"

#include <string>

namespace VW 
{
  namespace io 
  {
    struct io_adapter
    {
      io_adapter(bool is_resettable) : _is_resettable(is_resettable) {}
      virtual ~io_adapter(){};
      virtual size_t read(char* /*buffer*/, size_t /*num_bytes*/) = 0;
      virtual size_t write(const char* /*buffer*/, size_t /*num_bytes*/) = 0;
      virtual void flush() {}

      virtual void reset() { THROW("Reset not supported for this io_adapter"); }
      bool is_resettable() const { return _is_resettable; }
    private:
      bool _is_resettable;
    };

    enum class gzip_file_mode
    {
      read,
      write
    };

    std::unique_ptr<io_adapter> open_file(std::string& file_path);
    std::unique_ptr<io_adapter> open_file(const char* file_path);
    std::unique_ptr<io_adapter> open_compressed_file(std::string& file_path, gzip_file_mode mode);
    std::unique_ptr<io_adapter> open_compressed_file(const char* file_path, gzip_file_mode mode);
    std::unique_ptr<io_adapter> open_compressed_stdio();
    std::unique_ptr<io_adapter> open_stdio();
    std::unique_ptr<io_adapter> take_ownership_of_socket(int fd);
    std::unique_ptr<io_adapter> create_vector_buffer();
    std::unique_ptr<io_adapter> create_vector_buffer(const char* data, size_t len);
  }
}

using namespace VW::io;
