/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef IOBUF_H
#define IOBUF_H


#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include "v_array.h"
#include<iostream>
#include <errno.h>

using namespace std;

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
    space.resize(s);
    current = 0;
    count = 0;
    endloaded = space.begin;
  }

  virtual int open_file(const char* name, bool stdin_off, int flag=READ){

    int ret = -1;
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
      else if (!stdin_off)
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
      else
	{
	  cout << "can't open: " << name << " for read or write, exiting on error" << strerror(errno) << endl;
	  throw exception();
	}
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
      ret = -1;
    }

    return ret;
  }

  virtual void reset_file(int f){
#ifdef _WIN32
	_lseek(f, 0, SEEK_SET);
#else
    lseek(f, 0, SEEK_SET);
#endif
    endloaded = space.begin;
    space.end = space.begin;
  }

  io_buf() {
    init();
  }

  virtual ~io_buf(){
    files.delete_v();
    space.delete_v();
  }

  void set(char *p){space.end = p;}

  virtual ssize_t read_file(int f, void* buf, size_t nbytes){
#ifdef _WIN32
	  return _read(f, buf, (unsigned int)nbytes); 
#else
	  return read(f, buf, (unsigned int)nbytes); 
#endif
  }

  size_t fill(int f) {
    if (space.end_array - endloaded == 0)
      {
	size_t offset = endloaded - space.begin;
	space.resize(2 * (space.end_array - space.begin));
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
#ifdef _WIN32
    return _write(f, buf, (unsigned int)nbytes);
#else
    return write(f, buf, (unsigned int)nbytes);
#endif
  }

  virtual void flush() {
	  if (write_file(files[0], space.begin, space.size()) != (int) space.size())
      std::cerr << "error, failed to write example\n";
    space.end = space.begin; }

  virtual bool close_file(){
    if(files.size()>0){
#ifdef _WIN32
		_close(files.pop());
#else
		close(files.pop());
#endif
      return true;
    }
    return false;
  }

  void close_files(){
    while(close_file());
  }
};

void buf_write(io_buf &o, char* &pointer, size_t n);
size_t buf_read(io_buf &i, char* &pointer, size_t n);
bool isbinary(io_buf &i);
size_t readto(io_buf &i, char* &pointer, char terminal);

//if read_message is null, just read it in.  Otherwise do a comparison and barf on read_message.
inline size_t bin_read_fixed(io_buf& i, char* data, size_t len, const char* read_message)
{
  if (len > 0)
    {
      char* p;
      size_t ret = buf_read(i,p,len);
      if (*read_message == '\0')
	memcpy(data,p,len);
      else
	if (memcmp(data,p,len) != 0)
	  {
	    cout << read_message << endl;
	    throw exception();
	  }
      return ret;
    }
  return 0;
}

inline size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = bin_read_fixed(i,(char*)&obj_len,sizeof(obj_len),"");
  if (obj_len > len || ret < sizeof(uint32_t))
    {
      cerr << "bad model format!" <<endl;
      throw exception();
    }
  ret += bin_read_fixed(i,data,obj_len,read_message);

  return ret;
}

inline size_t bin_write_fixed(io_buf& o, const char* data, uint32_t len)
{
  if (len > 0)
    {
      char* p;
      buf_write (o, p, len);
      memcpy (p, data, len);
    }
  return len;
}

inline size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  bin_write_fixed(o,(char*)&len, sizeof(len));
  bin_write_fixed(o,data,len);
  return (len + sizeof(len));
}

inline size_t bin_text_write(io_buf& io, char* data, uint32_t len, 
		      const char* text_data, uint32_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else 
    if (len > 0)
      return bin_write (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write(io_buf& io, char* data, uint32_t len, 
			 const char* read_message, bool read, 
			 const char* text_data, uint32_t text_len, bool text)
{
  if (read)
    return bin_read(io, data, len, read_message);
  else
    return bin_text_write(io,data,len, text_data, text_len, text);
}

inline size_t bin_text_write_fixed(io_buf& io, char* data, uint32_t len, 
		      const char* text_data, uint32_t text_len, bool text)
{
  if (text)
    return bin_write_fixed (io, text_data, text_len);
  else 
    return bin_write_fixed (io, data, len);
  return 0;
}

//a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write_fixed(io_buf& io, char* data, uint32_t len, 
			       const char* read_message, bool read, 
			       const char* text_data, uint32_t text_len, bool text)
{
  if (read)
    return bin_read_fixed(io, data, len, read_message);
  else
    return bin_text_write_fixed(io, data, len, text_data, text_len, text);
}

#endif
