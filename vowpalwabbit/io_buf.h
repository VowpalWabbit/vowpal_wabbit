/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include "v_array.h"
#include <iostream>
#include <sstream>
#include <errno.h>
#include <stdexcept>
#include "hash.h"
#include "vw_exception.h"
#include "vw_validate.h"

#ifndef O_LARGEFILE  // for OSX
#define O_LARGEFILE 0
#endif

#ifdef _WIN32
#define ssize_t int64_t
#include <io.h>
#include <sys/stat.h>
#endif

#include "io_adapter.h"

/* The i/o buffer can be conceptualized as an array below:
**  _______________________________________________________________________________________
** |__________|__________|__________|__________|__________|__________|__________|__________|   **
** space.begin           space.head             space.end                       space.endarray **
**
** space.begin     = the beginning of the loaded values in the buffer
** space.head      = the end of the last-read point in the buffer
** space.end       = the end of the loaded values from file
** space.endarray  = the end of the allocated space for the array
**
** The values are ordered so that:
** space.begin <= space.head <= space.end <= space.endarray
**
** Initially space.begin == space.head since no values have been read.
**
** The interval [space.head, space.end] may be shifted down to space.begin
** if the requested number of bytes to be read is larger than the interval size.
** This is done to avoid reallocating arrays as much as possible.
*/

namespace VW {
  namespace io {
    std::unique_ptr<io_adapter> open_file(std::string& file_path)
    {
      return std::unique_ptr<io_adapter>(new file_adapter(file_path));
    }

    std::unique_ptr<io_adapter> open_stdio()
    {
      return std::unique_ptr<io_adapter>(new stdio_adapter());
    }
  }
}

class io_buf
{
  // used to check-sum i/o files for corruption detection
  bool _verify_hash;
  uint32_t _hash;

 public:
  v_array<char> space;  // space.begin = beginning of loaded values.  space.end = end of read or written values from/to
                        // the buffer.
  v_array<io_adapter*> files;
  size_t count;    // maximum number of file descriptors.
  size_t current;  // file descriptor currently being used.
  char* head;
  v_array<char> currentname;
  v_array<char> finalname;

  static constexpr int READ = 1;
  static constexpr int WRITE = 2;

  void init()
  {
    space = v_init<char>();
    currentname = v_init<char>();
    finalname = v_init<char>();
    size_t s = 1 << 16;
    space.resize(s);
    current = 0;
    count = 0;
    head = space.begin();
    _verify_hash = false;
    _hash = 0;
  }

  void verify_hash(bool verify)
  {
    _verify_hash = verify;
    // reset the hash so that the io_buf can be re-used for loading
    // as it is done for Reload()
    if (!verify)
      _hash = 0;
  }

  uint32_t hash()
  {
    if (!_verify_hash)
      THROW("HASH WAS NOT CALCULATED");
    return _hash;
  }

   virtual int open_file(const char* name, bool stdin_off, int flag = READ)
  {
    // TODO throw on fail or catch and return
    int ret = -1;
    switch (flag)
    {
      case READ:
        if (*name != '\0')
        {
          files.push_back(new file_adapter(name));
        }
        else if (!stdin_off)
          files.push_back(new stdio_adapter());
        break;

      case WRITE:
        files.push_back(new file_adapter(name));
        break;

      default:
        std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
        ret = -1;
    }
    if (ret == -1 && *name != '\0')
      THROWERRNO("can't open: " << name);
    return ret;
  }

  virtual void reset_file(io_adapter* f)
  {
    f->reset();
    space.end() = space.begin();
    head = space.begin();
  }

  io_buf() { init(); }

  virtual ~io_buf()
  {
    files.delete_v();
    space.delete_v();
  }

  void set(char* p) { head = p; }

  virtual size_t num_files() { return files.size(); }

  virtual ssize_t read_file(io_adapter* f, void* buf, size_t nbytes) { return f->read((char*)buf, nbytes); }

  ssize_t fill(io_adapter* f)
  {  // if the loaded values have reached the allocated space
    if (space.end_array - space.end() == 0)
    {  // reallocate to twice as much space
      size_t head_loc = head - space.begin();
      space.resize(2 * (space.end_array - space.begin()));
      head = space.begin() + head_loc;
    }
    // read more bytes from file up to the remaining allocated space
    ssize_t num_read = read_file(f, space.end(), space.end_array - space.end());
    if (num_read >= 0)
    {  // if some bytes were actually loaded, update the end of loaded values
      space.end() = space.end() + num_read;
      return num_read;
    }
    else
      return 0;
  }

  virtual ssize_t write_file(io_adapter* f, void* buf, size_t nbytes) { return f->write(static_cast<const char*>(buf), nbytes); }

  virtual void flush()
  {
    if (files.size() > 0)
    {
      if (write_file(files[0], space.begin(), head - space.begin()) != (int)(head - space.begin()))
        std::cerr << "error, failed to write example\n";
      head = space.begin();
    }
  }

  virtual bool close_file()
  {
    if (files.size() > 0)
    {
      delete files.pop();
      return true;
    }
    return false;
  }

  virtual bool compressed() { return false; }

  void close_files()
  {
    while (close_file())
      ;
  }

  void buf_write(char*& pointer, size_t n);
  size_t buf_read(char*& pointer, size_t n);

  // if read_message is null, just read it in.  Otherwise do a comparison and barf on read_message.
  size_t bin_read_fixed(char* data, size_t len, const char* read_message)
  {
    if (len > 0)
    {
      char* p;
      // if the model is corrupt the number of bytes can be less then specified (as there isn't enought data available
      // in the file)
      len = buf_read(p, len);

      // compute hash for check-sum
      if (_verify_hash)
        _hash = (uint32_t)uniform_hash(p, len, _hash);

      if (*read_message == '\0')
        memcpy(data, p, len);
      else if (memcmp(data, p, len) != 0)
        THROW(read_message);
      return len;
    }
    return 0;
  }

  size_t bin_write_fixed(const char* data, size_t len)
  {
    if (len > 0)
    {
      char* p;
      buf_write(p, len);

      memcpy(p, data, len);

      // compute hash for check-sum
      if (_verify_hash)
        _hash = (uint32_t)uniform_hash(p, len, _hash);
    }
    return len;
  }
};

bool isbinary(io_buf& i);
size_t readto(io_buf& i, char*& pointer, char terminal);

inline size_t bin_read(io_buf& i, char* data, size_t len, const char* read_message)
{
  uint32_t obj_len;
  size_t ret = i.bin_read_fixed((char*)&obj_len, sizeof(obj_len), "");
  if (obj_len > len || ret < sizeof(uint32_t))
    THROW("bad model format!");

  ret += i.bin_read_fixed(data, obj_len, read_message);

  return ret;
}

inline size_t bin_write(io_buf& o, const char* data, uint32_t len)
{
  o.bin_write_fixed((char*)&len, sizeof(len));
  o.bin_write_fixed(data, len);
  return (len + sizeof(len));
}

inline size_t bin_text_write(io_buf& io, char* data, size_t len, std::stringstream& msg, bool text)
{
  if (text)
  {
    size_t temp = io.bin_write_fixed(msg.str().c_str(), msg.str().size());
    msg.str("");
    return temp;
  }
  else
    return bin_write(io, data, (uint32_t)len);
  return 0;
}

// a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  if (read)
    return bin_read(io, data, len, read_message);
  else
    return bin_text_write(io, data, len, msg, text);
}

inline size_t bin_text_write_fixed(io_buf& io, char* data, size_t len, std::stringstream& msg, bool text)
{
  if (text)
  {
    size_t temp = io.bin_write_fixed(msg.str().c_str(), msg.str().size());
    msg.str("");
    return temp;
  }
  else
    return io.bin_write_fixed(data, len);
  return 0;
}

// a unified function for read(in binary), write(in binary), and write(in text)
inline size_t bin_text_read_write_fixed(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  if (read)
    return io.bin_read_fixed(data, len, read_message);
  else
    return bin_text_write_fixed(io, data, len, msg, text);
}

inline size_t bin_text_read_write_fixed_validated(
    io_buf& io, char* data, size_t len, const char* read_message, bool read, std::stringstream& msg, bool text)
{
  size_t nbytes = bin_text_read_write_fixed(io, data, len, read_message, read, msg, text);
  if (read && len > 0)  // only validate bytes read/write if expected length > 0
  {
    if (nbytes == 0)
    {
      THROW("Unexpected end of file encountered.");
    }
  }
  return nbytes;
}

#define writeit(what, str)                                                                  \
  do                                                                                        \
  {                                                                                         \
    msg << str << " = " << what << " ";                                                     \
    bin_text_read_write_fixed(model_file, (char*)&what, sizeof(what), "", read, msg, text); \
  } while (0);

#define writeitvar(what, str, mywhat)                                                           \
  auto mywhat = (what);                                                                         \
  do                                                                                            \
  {                                                                                             \
    msg << str << " = " << mywhat << " ";                                                       \
    bin_text_read_write_fixed(model_file, (char*)&mywhat, sizeof(mywhat), "", read, msg, text); \
  } while (0);
