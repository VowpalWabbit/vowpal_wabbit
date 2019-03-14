#include "vw.h"
#include <vector>

class vector_io_buf : public io_buf
{
public:
  std::vector<char> _buffer;

  vector_io_buf();

  virtual int open_file(const char* name, bool stdin_off, int flag = READ);

  virtual void reset_file(int f);

  virtual ssize_t read_file(int f, void* buf, size_t nbytes);

  virtual size_t num_files();

  virtual ssize_t write_file(int file, const void* buf, size_t nbytes);

  virtual bool compressed();

  virtual bool close_file();
};