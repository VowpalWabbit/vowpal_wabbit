// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#include "io_buf.h"
#include "v_array.h"
#include <vector>
#include <cstdio>

#if (ZLIB_VERNUM < 0x1252)
typedef void* gzFile;
#else
struct gzFile_s;
typedef struct gzFile_s* gzFile;
#endif

class comp_io_buf : public io_buf
{
 public:
  std::vector<gzFile> gz_files;

  int open_file(const char* name, bool stdin_off, int flag) override;

  void reset_file(int f) override;

  ssize_t read_file(int f, void* buf, size_t nbytes) override;

  size_t num_files() override;

  ssize_t write_file(int file, const void* buf, size_t nbytes) override;

  bool compressed() override;

  void flush() override;

  bool close_file() override;
};
