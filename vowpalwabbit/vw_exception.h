// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#ifndef VW_NOEXCEPT
#include <stdexcept>
#include <sstream>

#include <cstring>

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

namespace VW
{
class vw_exception : public std::exception
{
 private:
  // Source file exception was thrown in.
  const char* _file;

  std::string _message;

  // Line number exception was thrown in.
  int _line_number;

 public:
  vw_exception(const char* file, int lineNumber, std::string const& message)
      : _file(file), _message(message), _line_number(lineNumber)
  {
  }
  vw_exception(const vw_exception& ex) = default;
  vw_exception& operator=(const vw_exception& other) = default;
  vw_exception(vw_exception&& ex) noexcept = default;
  vw_exception& operator=(vw_exception&& other) noexcept = default;
  ~vw_exception() noexcept = default;

  const char* what() const noexcept override { return _message.c_str(); }
  const char* Filename() const { return _file; }
  int LineNumber() const { return _line_number; }
};

class vw_argument_disagreement_exception : public vw_exception
{
 public:
  vw_argument_disagreement_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_argument_disagreement_exception(const vw_argument_disagreement_exception& ex) = default;
  vw_argument_disagreement_exception& operator=(const vw_argument_disagreement_exception& other) = default;
  vw_argument_disagreement_exception(vw_argument_disagreement_exception&& ex) noexcept = default;
  vw_argument_disagreement_exception& operator=(vw_argument_disagreement_exception&& other) noexcept = default;
  ~vw_argument_disagreement_exception() noexcept override = default;
};

class vw_argument_invalid_value_exception : public vw_exception
{
 public:
  vw_argument_invalid_value_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_argument_invalid_value_exception(const vw_argument_invalid_value_exception& ex) = default;
  vw_argument_invalid_value_exception& operator=(const vw_argument_invalid_value_exception& other) = default;
  vw_argument_invalid_value_exception(vw_argument_invalid_value_exception&& ex) noexcept = default;
  vw_argument_invalid_value_exception& operator=(vw_argument_invalid_value_exception&& other) noexcept = default;
  ~vw_argument_invalid_value_exception() noexcept override = default;
};

class vw_unrecognised_option_exception : public vw_exception
{
 public:
  vw_unrecognised_option_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_unrecognised_option_exception(const vw_unrecognised_option_exception& ex) = default;
  vw_unrecognised_option_exception& operator=(const vw_unrecognised_option_exception& other) = default;
  vw_unrecognised_option_exception(vw_unrecognised_option_exception&& ex) noexcept = default;
  vw_unrecognised_option_exception& operator=(vw_unrecognised_option_exception&& other) noexcept = default;
  ~vw_unrecognised_option_exception() noexcept override = default;
};

class strict_parse_exception : public vw_exception
{
 public:
  strict_parse_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  strict_parse_exception(const strict_parse_exception& ex) = default;
  strict_parse_exception& operator=(const strict_parse_exception& other) = default;
  strict_parse_exception(strict_parse_exception&& ex) noexcept = default;
  strict_parse_exception& operator=(strict_parse_exception&& other) noexcept = default;
  ~strict_parse_exception() noexcept override = default;
};

#ifdef _WIN32
void vw_trace(const char* filename, int linenumber, const char* fmt, ...);

// useful when hunting down release mode bugs
#define VW_TRACE(fmt, ...) VW::vw_trace(__FILE__, __LINE__, fmt, __VA_ARGS__)

struct StopWatchData;

class StopWatch
{
  StopWatchData* data;

 public:
  StopWatch();
  ~StopWatch();

  double MilliSeconds() const;
};

// Equivalent to System::Diagnostics::Debugger::Launch();
bool launchDebugger();

#define THROWERRNO(args)                                         \
  {                                                              \
    std::stringstream __msg;                                     \
    __msg << args;                                               \
    char __errmsg[256];                                          \
    if (strerror_s(__errmsg, sizeof __errmsg, errno) != 0)       \
      __msg << ", errno = unknown";                              \
    else                                                         \
      __msg << ", errno = " << __errmsg;                         \
    throw VW::vw_exception(__FILENAME__, __LINE__, __msg.str()); \
  }
#else
#define THROWERRNO(args)                                         \
  {                                                              \
    std::stringstream __msg;                                     \
    __msg << args;                                               \
    char __errmsg[256];                                          \
    if (strerror_r(errno, __errmsg, sizeof __errmsg) != 0)       \
      __msg << "errno = unknown";                                \
    else                                                         \
      __msg << "errno = " << __errmsg;                           \
    throw VW::vw_exception(__FILENAME__, __LINE__, __msg.str()); \
  }
#endif

// ease error handling and also log filename and line number
#define THROW(args)                                              \
  {                                                              \
    std::stringstream __msg;                                     \
    __msg << args;                                               \
    throw VW::vw_exception(__FILENAME__, __LINE__, __msg.str()); \
  }

#define THROW_EX(ex, args)                         \
  {                                                \
    std::stringstream __msg;                       \
    __msg << args;                                 \
    throw ex(__FILENAME__, __LINE__, __msg.str()); \
  }
}  // namespace VW

#define VW_ASSERT(condition, args) \
  if (!(condition))                \
  {                                \
    THROW(args);                   \
  }

#endif

#define EXPAND(x) x
#define GET_MACRO(_1, _2, NAME, ...) NAME
// UNUSED is used to silence the warning: warning: ISO C++11 requires at least one argument for the "..." in a variadic
// macro
#define THROW_OR_RETURN(...) \
  EXPAND(GET_MACRO(__VA_ARGS__, THROW_OR_RETURN_NORMAL, THROW_OR_RETURN_VOID, UNUSED)(__VA_ARGS__))

#ifdef VW_NOEXCEPT

#define THROW_OR_RETURN_NORMAL(args, retval) \
  do                                         \
  {                                          \
    return retval;                           \
  } while (0)

#define THROW_OR_RETURN_VOID(args) \
  do                               \
  {                                \
    return;                        \
  } while (0)

#else  // VW_NOEXCEPT defined

#define THROW_OR_RETURN_NORMAL(args, retval)                  \
  do                                                          \
  {                                                           \
    std::stringstream __msgA;                                 \
    __msgA << args;                                           \
    throw VW::vw_exception(__FILE__, __LINE__, __msgA.str()); \
  } while (0)

#define THROW_OR_RETURN_VOID(args)                            \
  do                                                          \
  {                                                           \
    std::stringstream __msgB;                                 \
    __msgB << args;                                           \
    throw VW::vw_exception(__FILE__, __LINE__, __msgB.str()); \
  } while (0)

#endif
#define _UNUSED(x) ((void)(x))

#define DBG(x)                                                                                                      \
  do                                                                                                                \
  {                                                                                                                 \
    std::cerr << "(" << __FILENAME__ << ":" << __LINE__ << "," << __func__ << ") " << #x << ": " << x << std::endl; \
  } while (0)
