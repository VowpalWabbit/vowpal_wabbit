/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once
#include <stdexcept>
#include <sstream>

#ifndef _NOEXCEPT
// _NOEXCEPT is required on Mac OS
// making sure other platforms don't barf
#define _NOEXCEPT throw ()
#endif

namespace VW {

class vw_exception : public std::exception
{
private:
  // source file exception was thrown
  const char* file;

  std::string message;

  // line number exception was thrown
  int lineNumber;
public:
  vw_exception(const char* file, int lineNumber, std::string message);

  vw_exception(const vw_exception& ex);

  ~vw_exception() _NOEXCEPT;

  virtual const char* what() const _NOEXCEPT;

  const char* Filename() const;

  int LineNumber() const;
};

#ifdef _WIN32
  void vw_trace(const char* filename, int linenumber, const char* fmt, ...);

  // useful when hunting down release mode bugs
#define VW_TRACE(fmt, ...) VW::vw_trace(__FILE__, __LINE__, fmt, __VA_ARGS__)


#define THROWERRNO(args) \
  { \
    std::stringstream __msg; \
    __msg << args; \
    char __errmsg[256]; \
    if (strerror_s(__errmsg, sizeof __errmsg, errno) != 0) \
      __msg << ", errno = unknown"; \
    else \
      __msg << ", errno = " << __errmsg; \
    throw VW::vw_exception(__FILE__, __LINE__, __msg.str()); \
  }
#else
#define THROWERRNO(args) \
  { \
    std::stringstream __msg; \
    __msg << args; \
    char __errmsg[256]; \
    if (strerror_r(errno, __errmsg, sizeof __errmsg) != 0) \
      __msg << "errno = unknown"; \
    else \
      __msg << "errno = " << __errmsg; \
    throw VW::vw_exception(__FILE__, __LINE__, __msg.str()); \
  }
#endif

// ease error handling and also log filename and line number
#define THROW(args) \
  { \
    std::stringstream __msg; \
    __msg << args; \
    throw VW::vw_exception(__FILE__, __LINE__, __msg.str()); \
  }

}

#define VW_ASSERT(condition, args) \
  if (! (condition)) {             \
    THROW(args); \
  }
