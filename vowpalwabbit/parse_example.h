/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include <stdint.h>
#include "memory.h"
#include "parse_primitives.h"
#include "example.h"

//example processing

int read_features(void* a, example* ex);// read example from  preset buffers.
void read_line(vw& all, example* ex, char* line);//read example from the line.

inline char* copy_char(char* base)
{
  size_t len = 0;
  while (base[++len] != '\0');
  char* ret = calloc_or_die<char>(len);
  memcpy(ret,base,len);
  return ret;
}
