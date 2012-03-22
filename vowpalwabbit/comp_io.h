/*
 * Copyright (c) 2007 Yahoo! Inc.  All rights reserved.  The copyrights
 * embodied in the content of this file are licensed under the BSD
 * (revised) open source license
 *  */

#ifndef COMP_IO_BUF_H_
#define COMP_IO_BUF_H_

#include "io.h"
#include "v_array.h"
#include "zlib.h"

class comp_io_buf : public io_buf
{
public:
  v_array<gzFile> gz_files;

  comp_io_buf()
  {
    init();
  }

  virtual int open_file(const char* name, int flag=READ){
    gzFile fil;
    int ret = -1;
    switch(flag){
    case READ:
      fil = gzopen(name, "rb");
      if(fil!=NULL){
        push(gz_files,fil);
        ret = gz_files.index()-1;
        push(files,ret);
      }
      else
        ret = -1;
      break;

    case WRITE:
      fil = gzopen(name, "wb");
      if(fil!=NULL){
        push(gz_files,fil);
        ret = gz_files.index()-1;
        push(files,ret);
      }
      else
        ret = -1;
      break;

    default:
      std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
      ret = -1;
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
    int num_read = gzread(fil, buf, nbytes);
    return (num_read > 0) ? num_read : 0;
  }

  virtual inline ssize_t write_file(int f, const void* buf, size_t nbytes)
  {
    gzFile fil = gz_files[f];
    int num_written = gzwrite(fil, buf, nbytes);
    return (num_written > 0) ? num_written : 0;
  }

  virtual void flush()
  {
    if (write_file(files[0], space.begin, space.index()) != (int) ((space.index())))
      std::cerr << "error, failed to write to cache\n";
    space.end = space.begin;
  }

  virtual bool close_file(){
    gzFile fil;
    if(files.index()>0){
      fil = gz_files[files.pop()];
      gzclose(fil);
      free(gz_files.begin);
      return true;
    }
    return false;
  }
};

#endif /* COMP_IO_BUF_H_ */
