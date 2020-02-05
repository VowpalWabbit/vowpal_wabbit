// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "clr_io_memory.h"
#include <algorithm>

namespace VW
{
clr_io_memory_buf::clr_io_memory_buf()
{ files.push_back(0);
  m_iterator = m_data.begin();
}

int clr_io_memory_buf::open_file(const char* name, bool stdin_off, int flag)
{ m_iterator = m_data.begin();
  return 0;
}

void clr_io_memory_buf::reset_file(int f)
{ size_t count = m_data.size();
  m_iterator = m_data.begin();
}

ssize_t clr_io_memory_buf::read_file(int f, void* buf, size_t nbytes)
{ size_t left_over = std::min(nbytes, static_cast<size_t>(m_data.end() - m_iterator));

  if (left_over == 0)
    return 0;

  memcpy_s(buf, nbytes, &*m_iterator, left_over);

  m_iterator += left_over;

  return left_over;
}

size_t clr_io_memory_buf::num_files()
{ return 1;
}

ssize_t clr_io_memory_buf::write_file(int file, const void* buf, size_t nbytes)
{ m_data.insert(m_data.end(), (char*)buf, (char*)buf + nbytes);
  return nbytes;
}

bool clr_io_memory_buf::compressed()
{ return false;
}

bool clr_io_memory_buf::close_file()
{ return true;
}
}
