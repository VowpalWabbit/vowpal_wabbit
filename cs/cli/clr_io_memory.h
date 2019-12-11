// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "io_buf.h"

#include <vector>

namespace VW
{
/// <summary>
/// IO Buffer keeping data in memory. Used by VowpalWabbit::Reload.
/// </summary>
class clr_io_memory_buf : public io_buf
{
private:
  std::vector<char> m_data;

  std::vector<char>::const_iterator m_iterator;

public:
  /// <summary>
  /// Initializes a new <see cref="clr_io_memory_buf"/> instance.
  /// </summary>
  clr_io_memory_buf();

  virtual int open_file(const char* name, bool stdin_off, int flag = READ);

  virtual void reset_file(int f);

  virtual ssize_t read_file(int f, void* buf, size_t nbytes);

  virtual size_t num_files();

  virtual ssize_t write_file(int file, const void* buf, size_t nbytes);

  virtual bool compressed();

  virtual bool close_file();
};
}
