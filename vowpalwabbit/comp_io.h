/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#include "io_buf.h"
#include "v_array.h"
#include "zlib.h"
#include <stdio.h>

class comp_io_buf : public io_buf
{
public:
  vector<gzFile> gz_files;

  virtual int open_file(const char* name, bool stdin_off, int flag=READ){
    gzFile fil=NULL;
    int ret = -1;
    switch(flag){
    case READ:
      if (*name != '\0')
	fil = gzopen(name, "rb");
      else if (!stdin_off)
#ifdef _WIN32
	fil = gzdopen(_fileno(stdin), "rb");
#else
       fil = gzdopen(fileno(stdin), "rb");
#endif
       if(fil!=NULL){
	 gz_files.push_back(fil);
	 ret = (int)gz_files.size()-1;
	 files.push_back(ret);
       }
      break;

    case WRITE:
      fil = gzopen(name, "wb");
      if(fil!=NULL){
        gz_files.push_back(fil);
        ret = (int)gz_files.size()-1;
	files.push_back(ret);
      }
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
    }
    return ret;
  }

  virtual void reset_file(int f){
    gzFile fil = gz_files[f];
    gzseek(fil, 0, SEEK_SET);
    endloaded = space.begin;
    space.end = space.begin;
  }

  virtual ssize_t read_file(int f, void* buf, size_t nbytes)
  {
    gzFile fil = gz_files[f];
    int num_read = gzread(fil, buf, (unsigned int)nbytes);
    return (num_read > 0) ? num_read : 0;
  }

  virtual size_t num_files(){ return gz_files.size();}

  virtual inline ssize_t write_file(int file, const void* buf, size_t nbytes)
  {
    int num_written = gzwrite(gz_files[file], buf, (unsigned int)nbytes);
    return (num_written > 0) ? num_written : 0;
  }

  virtual bool compressed() { return true; }

  virtual void flush()
  {
    if (write_file(0, space.begin, space.size()) != (int) ((space.size())))
      std::cerr << "error, failed to write to cache\n";
    space.end = space.begin;
  }

  virtual bool close_file(){
    if(gz_files.size()>0){
      gzclose(gz_files.back());
      gz_files.pop_back();
      if (files.size() > 0)
	files.pop();
      return true;
    }
    return false;
  }
};
