/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

/* 
Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef IO_H__
#define IO_H__

#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "v_array.h"

using namespace std;

class io_buf {
 public:
  v_array<char > space;
  int file;
  char* endloaded;
  string currentname;
  string finalname;
  
  io_buf() {
    size_t s = 1 << 16;
    reserve(space, s); 
    endloaded = space.begin;}
  void set(char *p){space.end = p;}
  size_t fill() {
    if (space.end_array - endloaded == 0)
      {
	size_t offset = endloaded - space.begin;
	reserve(space, 2 * (space.end_array - space.begin));
	endloaded = space.begin+offset;
      }
    ssize_t num_read = read(file, endloaded, space.end_array - endloaded);
    endloaded = endloaded+num_read;
    return num_read;
  }
  void flush() { 
    if (write(file, space.begin, space.index()) != (int) space.index())
      cerr << "error, failed to write to cache\n";
    space.end = space.begin; fsync(file); }
};

void buf_write(io_buf &o, char* &pointer, int n);
unsigned int buf_read(io_buf &i, char* &pointer, int n);
size_t readto(io_buf &i, char* &pointer, char terminal);
#endif
