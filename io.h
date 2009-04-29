/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef IO_H__
#define IO_H__

#include <iostream.h>
#include <errno.h>
#include "stack.h"

using namespace std;

class io_buf {
 public:
  v_array<char > space;
  int file;
  string currentname;
  string finalname;
  
  io_buf() {alloc(space, 1 << 16); }
  void set(char *p){space.end = p;}
  void fill(int n) {alloc(space, 
			  n + read(file, space.begin+n, space.end_array - space.begin -n));}
  void flush() { 
    if (write(file, space.begin, space.index()) != (int) space.index())
      cerr << "error, failed to write to cache\n";
    space.end = space.begin; fsync(file); }
};

void buf_write(io_buf &o, char* &pointer, int n);
unsigned int buf_read(io_buf &i, char* &pointer, int n);
#endif
