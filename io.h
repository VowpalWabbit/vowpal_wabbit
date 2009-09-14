/*
Copyright (c) 2009 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */

#ifndef IO_H__
#define IO_H__

#include "v_array.h"
#include<iostream>
using namespace std;

class io_buf {
 public:
  v_array<char> space; //space.begin = beginning of loaded values.  space.end = end of read or written values.
  v_array<int> files;
  size_t current; //file descriptor currently being used.
  char* endloaded; //end of loaded values
  v_array<char> currentname;
  v_array<char> finalname;
  
  io_buf() {
    size_t s = 1 << 16;
    reserve(space, s); 
    current = 0;
    endloaded = space.begin;
  }
  void set(char *p){space.end = p;}
  size_t fill(int f) {
    if (space.end_array - endloaded == 0)
      {
	size_t offset = endloaded - space.begin;
	reserve(space, 2 * (space.end_array - space.begin));
	endloaded = space.begin+offset;
      }
    ssize_t num_read = read(f, endloaded, space.end_array - endloaded);
    if (num_read >= 0)
      {
	endloaded = endloaded+num_read;
	return num_read;
      }
    else
      return 0;
  }
  void flush() { 
    if (write(files[0], space.begin, space.index()) != (int) space.index())
      cerr << "error, failed to write example\n";
    space.end = space.begin; fsync(files[0]); }
};

void buf_write(io_buf &o, char* &pointer, int n);
unsigned int buf_read(io_buf &i, char* &pointer, int n);
size_t readto(io_buf &i, char* &pointer, char terminal);
#endif
