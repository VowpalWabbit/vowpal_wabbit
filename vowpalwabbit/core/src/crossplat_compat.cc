// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/crossplat_compat.h"

#ifdef _WIN32
#  include <process.h>
#else
#  include <unistd.h>
#endif

int VW::string_cpy(char* dest, size_t dest_size, const char* src)
{
#ifdef _WIN32
  // strcpy_s returns an errno_t
  return strcpy_s(dest, dest_size, src);
#else
  (void)dest_size;  // unused here
  strcpy(dest, src);
  return 0;
#endif
}

int VW::file_open(FILE** pf, const char* filename, const char* mode)
{
#ifdef _WIN32
  // fopen_s returns an errno_t
  return fopen_s(pf, filename, mode);
#else
  *pf = fopen(filename, mode);
  if (*pf == nullptr) { return -1; }
  return 0;
#endif
}

int VW::get_pid()
{
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}
