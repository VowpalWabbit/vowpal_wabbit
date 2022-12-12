// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once
#ifndef VW_NOEXCEPT

#  include "vw/common/future_compat.h"
#  include "vw/common/vwvis.h"

#  include <cstring>
#  include <exception>
#  include <string>

// We want the exception types to marked as visible but not dll export
#  ifndef _WIN32
#    define VW_EXPORT_EXCEPTION VW_DLL_PUBLIC
#  else
#    define VW_EXPORT_EXCEPTION
#  endif  // _WIN32

#  ifdef _WIN32
#    define VW_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#  else
#    define VW_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#  endif

#  define VW_LINE __LINE__

namespace VW
{
class VW_EXPORT_EXCEPTION vw_exception : public std::exception
{
public:
  vw_exception(const char* file, int lineNumber, std::string const& message)
      : _file(file), _message(message), _line_number(lineNumber)
  {
  }
  vw_exception(const vw_exception& ex) = default;
  vw_exception& operator=(const vw_exception& other) = default;
  vw_exception(vw_exception&& ex) = default;
  vw_exception& operator=(vw_exception&& other) = default;
  ~vw_exception() noexcept override = default;

  const char* what() const noexcept override { return _message.c_str(); }
  VW_DEPRECATED("VW::vw_exception::Filename renamed to VW::vw_exception::filename")
  const char* Filename() const { return _file; }  // NOLINT
  const char* filename() const { return _file; }
  VW_DEPRECATED("VW::vw_exception::LineNumber renamed to VW::vw_exception::line_number")
  int LineNumber() const { return _line_number; }  // NOLINT
  int line_number() const { return _line_number; }

private:
  // Source file exception was thrown in.
  const char* _file;

  std::string _message;

  // Line number exception was thrown in.
  int _line_number;
};

class VW_EXPORT_EXCEPTION vw_argument_disagreement_exception : public vw_exception
{
public:
  vw_argument_disagreement_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_argument_disagreement_exception(const vw_argument_disagreement_exception& ex) = default;
  vw_argument_disagreement_exception& operator=(const vw_argument_disagreement_exception& other) = default;
  vw_argument_disagreement_exception(vw_argument_disagreement_exception&& ex) = default;
  vw_argument_disagreement_exception& operator=(vw_argument_disagreement_exception&& other) = default;
  ~vw_argument_disagreement_exception() noexcept override = default;
};

class VW_EXPORT_EXCEPTION vw_argument_invalid_value_exception : public vw_exception
{
public:
  vw_argument_invalid_value_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_argument_invalid_value_exception(const vw_argument_invalid_value_exception& ex) = default;
  vw_argument_invalid_value_exception& operator=(const vw_argument_invalid_value_exception& other) = default;
  vw_argument_invalid_value_exception(vw_argument_invalid_value_exception&& ex) = default;
  vw_argument_invalid_value_exception& operator=(vw_argument_invalid_value_exception&& other) = default;
  ~vw_argument_invalid_value_exception() noexcept override = default;
};

class VW_EXPORT_EXCEPTION vw_unrecognised_option_exception : public vw_exception
{
public:
  vw_unrecognised_option_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  vw_unrecognised_option_exception(const vw_unrecognised_option_exception& ex) = default;
  vw_unrecognised_option_exception& operator=(const vw_unrecognised_option_exception& other) = default;
  vw_unrecognised_option_exception(vw_unrecognised_option_exception&& ex) = default;
  vw_unrecognised_option_exception& operator=(vw_unrecognised_option_exception&& other) = default;
  ~vw_unrecognised_option_exception() noexcept override = default;
};

class VW_EXPORT_EXCEPTION save_load_model_exception : public vw_exception
{
public:
  save_load_model_exception(const char* file, int lineNumber, const std::string& message)
      : vw_exception(file, lineNumber, message)
  {
  }

  save_load_model_exception(const save_load_model_exception& ex) = default;
  save_load_model_exception& operator=(const save_load_model_exception& other) = default;
  save_load_model_exception(save_load_model_exception&& ex) = default;
  save_load_model_exception& operator=(save_load_model_exception&& other) = default;
  ~save_load_model_exception() noexcept override = default;
};

class VW_EXPORT_EXCEPTION strict_parse_exception : public vw_exception
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
}  // namespace VW
#endif
