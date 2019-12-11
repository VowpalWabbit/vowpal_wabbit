// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#define NOMINMAX
#include "zlib.h"
#include "comp_io.h"

int comp_io_buf::open_file(const char* name, bool stdin_off, int flag)
{
  gzFile fil = nullptr;
  int ret = -1;
  switch (flag)
  {
    case READ:
      if (*name != '\0')
        fil = gzopen(name, "rb");
      else if (!stdin_off)
#ifdef _WIN32
        fil = gzdopen(_fileno(stdin), "rb");
#else
        fil = gzdopen(fileno(stdin), "rb");
#endif
      if (fil != nullptr)
      {
        gz_files.push_back(fil);
        ret = (int)gz_files.size() - 1;
        files.push_back(ret);
      }
      break;

    case WRITE:
      fil = gzopen(name, "wb");
      if (fil != nullptr)
      {
        gz_files.push_back(fil);
        ret = (int)gz_files.size() - 1;
        files.push_back(ret);
      }
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
  }
  return ret;
}

void comp_io_buf::reset_file(int f)
{
  gzFile fil = gz_files[f];
  gzseek(fil, 0, SEEK_SET);
  space.end() = space.begin();
  head = space.begin();
}

ssize_t comp_io_buf::read_file(int f, void* buf, size_t nbytes)
{
  gzFile fil = gz_files[f];
  int num_read = gzread(fil, buf, (unsigned int)nbytes);
  return (num_read > 0) ? num_read : 0;
}

size_t comp_io_buf::num_files() { return gz_files.size(); }

ssize_t comp_io_buf::write_file(int file, const void* buf, size_t nbytes)
{
  int num_written = gzwrite(gz_files[file], buf, (unsigned int)nbytes);
  return (num_written > 0) ? num_written : 0;
}

bool comp_io_buf::compressed() { return true; }

void comp_io_buf::flush()
{
  if (write_file(0, space.begin(), head - space.end()) != (int)((head - space.end())))
    std::cerr << "error, failed to write to cache\n";
  head = space.begin();
}

bool comp_io_buf::close_file()
{
  if (!gz_files.empty())
  {
    gzclose(gz_files.back());
    gz_files.pop_back();
    if (!files.empty())
      files.pop();
    return true;
  }
  return false;
}
