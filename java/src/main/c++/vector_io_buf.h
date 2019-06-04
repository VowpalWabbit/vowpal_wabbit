#include "vw.h"
#include <vector>

/**
 * io_buf wrapper around STL vector.
 */
class vector_io_buf : public io_buf
{
  std::vector<char>::iterator _iterator;

 public:
  std::vector<char> _buffer;

  vector_io_buf();

  vector_io_buf(const char* data, size_t len);

  virtual int open_file(const char* name, bool stdin_off, int flag = READ);

  virtual void reset_file(int f);

  virtual ssize_t read_file(int f, void* buf, size_t nbytes);

  virtual size_t num_files();

  virtual ssize_t write_file(int file, const void* buf, size_t nbytes);

  virtual bool compressed();

  virtual bool close_file();
};