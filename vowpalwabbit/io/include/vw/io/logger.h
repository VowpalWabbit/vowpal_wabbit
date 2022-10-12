// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <fmt/core.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#ifdef _M_CEE
#  pragma managed(push, off)
#  undef _M_CEE
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/null_sink.h>
#  include <spdlog/sinks/stdout_color_sinks.h>
#  include <spdlog/sinks/stdout_sinks.h>
#  include <spdlog/spdlog.h>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/null_sink.h>
#  include <spdlog/sinks/stdout_color_sinks.h>
#  include <spdlog/sinks/stdout_sinks.h>
#  include <spdlog/spdlog.h>
#endif

#include "vw/common/vw_exception.h"

// needed for custom types (like string_view)
#include <fmt/ostream.h>

namespace VW
{
namespace io
{
enum class output_location
{
  STDOUT,
  STDERR,
  COMPAT
};

output_location get_output_location(const std::string& name);

// There are global macros defined which conflict without the _LEVEL suffix.
enum class log_level
{
  TRACE_LEVEL = spdlog::level::trace,
  DEBUG_LEVEL = spdlog::level::debug,
  INFO_LEVEL = spdlog::level::info,
  WARN_LEVEL = spdlog::level::warn,
  ERROR_LEVEL = spdlog::level::err,
  CRITICAL_LEVEL = spdlog::level::critical,
  OFF_LEVEL = spdlog::level::off
};

log_level get_log_level(const std::string& level);

using logger_output_func_t = void (*)(void*, VW::io::log_level, const std::string&);
using logger_legacy_output_func_t = void (*)(void*, const std::string&);

namespace details
{
const constexpr char* DEFAULT_PATTERN = "%^[%l]%$ %v";
class logger_impl
{
public:
  std::unique_ptr<spdlog::logger> spdlog_stdout_logger;
  std::unique_ptr<spdlog::logger> spdlog_stderr_logger;
  size_t max_limit = SIZE_MAX;
  size_t log_count = 0;
  output_location location = output_location::COMPAT;

  logger_impl(std::unique_ptr<spdlog::logger> inner_stdout_logger, std::unique_ptr<spdlog::logger> inner_stderr_logger)
      : spdlog_stdout_logger(std::move(inner_stdout_logger)), spdlog_stderr_logger(std::move(inner_stderr_logger))
  {
    spdlog_stdout_logger->set_pattern(details::DEFAULT_PATTERN);
    spdlog_stdout_logger->set_level(spdlog::level::info);
    spdlog_stderr_logger->set_pattern(details::DEFAULT_PATTERN);
    spdlog_stderr_logger->set_level(spdlog::level::info);
  }

  template <typename FormatString, typename... Args>
  void err_info(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void err_warn(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void err_error(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void err_critical(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    // we ignore max_limit with critical log
    if (location == output_location::COMPAT) { spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...); }
    else if (location == output_location::STDERR)
    {
      spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...);
    }
    else
    {
      spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...);
    }
  }

  template <typename FormatString, typename... Args>
  void out_info(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void out_warn(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void out_error(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...); }
      else if (location == output_location::STDERR)
      {
        spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...);
      }
      else
      {
        spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename FormatString, typename... Args>
  void out_critical(const FormatString& fmt, Args&&... args)
  {
    log_count++;
    // we ignore max_limit with critical log
    if (location == output_location::COMPAT) { spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...); }
    else if (location == output_location::STDERR)
    {
      spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...);
    }
    else
    {
      spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...);
    }
  }
};

template <typename Mutex>
class function_ptr_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
  function_ptr_sink(void* context, logger_output_func_t func)
      : spdlog::sinks::base_sink<Mutex>(), _func(func), _context(context)
  {
  }

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    _func(_context, static_cast<log_level>(msg.level), fmt::to_string(formatted));
  }

  void flush_() override {}

  logger_output_func_t _func;
  void* _context;
};

// Same as above but ignores the log level.
template <typename Mutex>
class function_ptr_legacy_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
  function_ptr_legacy_sink(void* context, logger_legacy_output_func_t func)
      : spdlog::sinks::base_sink<Mutex>(), _func(func), _context(context)
  {
  }

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    _func(_context, fmt::to_string(formatted));
  }

  void flush_() override {}

  logger_legacy_output_func_t _func;
  void* _context;
};

}  // namespace details

class logger
{
public:
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_info(fmt, std::forward<Args>(args)...);
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_warn(fmt, std::forward<Args>(args)...);
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_error(fmt, std::forward<Args>(args)...);
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_critical(fmt, std::forward<Args>(args)...);
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_info(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_warn(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_error(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_critical(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_info(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_warn(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_error(fmt, std::forward<Args>(args)...);
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_critical(fmt, std::forward<Args>(args)...);
  }

  void set_level(log_level lvl);
  void set_max_output(size_t max);
  void set_location(output_location location);
  size_t get_log_count() const;
  void log_summary();

private:
  std::shared_ptr<details::logger_impl> _logger_impl;

  logger(std::shared_ptr<details::logger_impl> inner_logger) : _logger_impl(std::move(inner_logger)) {}

  friend logger create_default_logger();
  friend logger create_null_logger();
  friend logger create_custom_sink_logger(void* context, logger_output_func_t func);
  friend logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func);
};

inline logger create_default_logger()
{
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", stdout_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", stderr_sink))

          ));
}

inline logger create_null_logger()
{
  auto null_out_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  auto null_err_sink = std::make_shared<spdlog::sinks::null_sink_st>();
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", null_out_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stderr", null_err_sink))

          ));
}

inline logger create_custom_sink_logger(void* context, logger_output_func_t func)
{
  auto fptr_out_sink = std::make_shared<details::function_ptr_sink<std::mutex>>(context, func);
  auto fptr_err_sink = std::make_shared<details::function_ptr_sink<std::mutex>>(context, func);
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_out_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_err_sink))));
}

inline logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func)
{
  auto fptr_out_sink = std::make_shared<details::function_ptr_legacy_sink<std::mutex>>(context, func);
  auto fptr_err_sink = std::make_shared<details::function_ptr_legacy_sink<std::mutex>>(context, func);
  return logger(std::make_shared<details::logger_impl>(
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_out_sink)),
      std::unique_ptr<spdlog::logger>(new spdlog::logger("vowpal-stdout", fptr_err_sink))));
}

}  // namespace io
}  // namespace VW
