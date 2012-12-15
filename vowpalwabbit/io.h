/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef IO_H__
#define IO_H__


#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include "v_array.h"
#include<iostream>

#ifndef O_LARGEFILE //for OSX
#define O_LARGEFILE 0
#endif

#ifdef _WIN32
#define ssize_t size_t
#include <io.h>
#include <sys/stat.h>
#endif

class io_buf {
 public:
  v_array<char> space; //space.begin = beginning of loaded values.  space.end = end of read or written values.
  v_array<int> files;
  size_t count; // maximum number of file descriptors.
  size_t current; //file descriptor currently being used.
  char* endloaded; //end of loaded values
  v_array<char> currentname;
  v_array<char> finalname;
  
  static const int READ = 1;
  static const int WRITE = 2;

  void init(){
    size_t s = 1 << 16;
    reserve(space, s);
    current = 0;
    count = 0;
    endloaded = space.begin;
  }

  virtual int open_file(const char* name, int flag=READ){
	int ret;
    switch(flag){
    case READ:
      if (*name != '\0')
	{
#ifdef _WIN32
	  // _O_SEQUENTIAL hints to OS that we'll be reading sequentially, so cache aggressively.
	  _sopen_s(&ret, name, _O_RDONLY|_O_BINARY|_O_SEQUENTIAL, _SH_DENYWR, 0);
#else
	  ret = open(name, O_RDONLY|O_LARGEFILE);
#endif
	}
      else
#ifdef _WIN32
	ret = _fileno(stdin);
#else
	ret = fileno(stdin);
#endif
      if(ret!=-1)
	files.push_back(ret);
      break;

    case WRITE:
#ifdef _WIN32
		_sopen_s(&ret, name, _O_CREAT|_O_WRONLY|_O_BINARY|_O_TRUNC, _SH_DENYWR, _S_IREAD|_S_IWRITE);
#else
		ret = open(name, O_CREAT|O_WRONLY|O_LARGEFILE|O_TRUNC,0666);
#endif
      if(ret!=-1)
        files.push_back(ret);
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
      ret = -1;
    }
    return ret;
  }

  virtual void reset_file(int f){
    lseek(f, 0, SEEK_SET);
    endloaded = space.begin;
    space.end = space.begin;
  }

  io_buf() {
    init();
  }

  virtual ~io_buf(){
    free(files.begin);
    free(space.begin);
  }

  void set(char *p){space.end = p;}

  virtual ssize_t read_file(int f, void* buf, size_t nbytes){
	  return read(f, buf, (unsigned int)nbytes); 
  }

  size_t fill(int f) {
    if (space.end_array - endloaded == 0)
      {
	size_t offset = endloaded - space.begin;
	reserve(space, 2 * (space.end_array - space.begin));
	endloaded = space.begin+offset;
      }
    ssize_t num_read = read_file(f, endloaded, space.end_array - endloaded);
    if (num_read >= 0)
      {
	endloaded = endloaded+num_read;
	return num_read;
      }
    else
      return 0;
  }

  virtual ssize_t write_file(int f, const void* buf, size_t nbytes)
  {
    return write(f, buf, (unsigned int)nbytes);
  }

  virtual void flush() {
	  if (write_file(files[0], space.begin, space.size()) != (int) space.size())
      std::cerr << "error, failed to write example\n";
    space.end = space.begin; }

  virtual bool close_file(){
    if(files.size()>0){
      close(files.pop());
      return true;
    }
    return false;
  }

  void close_files(){
    while(close_file());
  }
};

void buf_write(io_buf &o, char* &pointer, int n);
size_t buf_read(io_buf &i, char* &pointer, size_t n);
bool isbinary(io_buf &i);
size_t readto(io_buf &i, char* &pointer, char terminal);
#endif
