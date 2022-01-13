// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include <string>
#include <utility>

#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <spdlog/spdlog.h>
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/stdout_sinks.h>
#  include <spdlog/sinks/null_sink.h>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <spdlog/spdlog.h>
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/stdout_sinks.h>
#  include <spdlog/sinks/null_sink.h>
#endif

// needed for custom types (like string_view)
#include <fmt/ostream.h>

#include "../vw_exception.h"

namespace VW
{
namespace io
{
enum class output_location
{
  out,
  err
};

output_location get_output_location(const std::string& name);

namespace details
{
const constexpr char* default_pattern = "[%l] %v";
struct logger_impl
{
  std::unique_ptr<spdlog::logger> _spdlog_stdout_logger;
  std::unique_ptr<spdlog::logger> _spdlog_stderr_logger;
  size_t _max_limit = SIZE_MAX;
  size_t _log_count = 0;
  output_location _location = output_location::out;

  logger_impl(std::unique_ptr<spdlog::logger> inner_stdout_logger, std::unique_ptr<spdlog::logger> inner_stderr_logger)
      : _spdlog_stdout_logger(std::move(inner_stdout_logger)), _spdlog_stderr_logger(std::move(inner_stderr_logger))
  {
    _spdlog_stdout_logger->set_pattern(details::default_pattern);
    _spdlog_stdout_logger->set_level(spdlog::level::info);
    _spdlog_stderr_logger->set_pattern(details::default_pattern);
    _spdlog_stderr_logger->set_level(spdlog::level::info);
  }

  template <typename FormatString, typename... Args>
  void info(const FormatString& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::err) { _spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...); }
      else
      {
        _spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void warn(const FormatString& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::err) { _spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...); }
      else
      {
        _spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void error(const FormatString& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::err) { _spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...); }
      else
      {
        _spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void critical(const FormatString& fmt, Args&&... args)
  {
    _log_count++;
    // we ignore max_limit with critical log
    if (_location == output_location::err) { _spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...); }
    else
    {
      _spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...);
    }
  }
};
}  // namespace details

enum class log_level
{
  trace = spdlog::level::trace,
  debug = spdlog::level::debug,
  info = spdlog::level::info,
  warn = spdlog::level::warn,
  error = spdlog::level::err,
  critical = spdlog::level::critical,
  off = spdlog::level::off
};

log_level get_log_level(const std::string& level);

struct logger
{
private:
  std::shared_ptr<details::logger_impl> _logger_impl;

  logger(std::shared_ptr<details::logger_impl> inner_logger) : _logger_impl(std::move(inner_logger)) {}

  friend logger create_default_logger();
  friend logger create_null_logger();
  friend logger create_custom_sink_logger(void* context, void (*func)(void*, const std::string&));

public:
  template <typename FormatString, typename... Args>
  void info(const FormatString& fmt, Args&&... args)
  {
    _logger_impl->info(fmt, std::forward<Args>(args)...);
  }

  template <typename FormatString, typename... Args>
  void warn(const FormatString& fmt, Args&&... args)
  {
    _logger_impl->warn(fmt, std::forward<Args>(args)...);
  }

  template <typename FormatString, typename... Args>
  void error(const FormatString& fmt, Args&&... args)
  {
    _logger_impl->error(fmt, std::forward<Args>(args)...);
  }

  template <typename FormatString, typename... Args>
  void critical(const FormatString& fmt, Args&&... args)
  {
    _logger_impl->critical(fmt, std::forward<Args>(args)...);
  }

  void set_level(log_level lvl);
  void set_max_output(size_t max);
  void set_location(output_location location);
  size_t get_log_count() const;
  void log_summary();
};

logger create_default_logger();
logger create_null_logger();
logger create_custom_sink_logger(void* context, void (*func)(void*, const std::string&));

}  // namespace io
}  // namespace VW
