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
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/null_sink.h>
#  include <spdlog/sinks/stdout_sinks.h>
#  include <spdlog/spdlog.h>
#  define _M_CEE 001
#  pragma managed(pop)
#else
#  include <spdlog/sinks/base_sink.h>
#  include <spdlog/sinks/null_sink.h>
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
  out,
  err,
  compat
};

output_location get_output_location(const std::string& name);

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

using logger_output_func_t = void (*)(void*, VW::io::log_level, const std::string&);
using logger_legacy_output_func_t = void (*)(void*, const std::string&);

namespace details
{
const constexpr char* default_pattern = "[%l] %v";
struct logger_impl
{
  std::unique_ptr<spdlog::logger> _spdlog_stdout_logger;
  std::unique_ptr<spdlog::logger> _spdlog_stderr_logger;
  size_t _max_limit = SIZE_MAX;
  size_t _log_count = 0;
  output_location _location = output_location::compat;

  logger_impl(std::unique_ptr<spdlog::logger> inner_stdout_logger, std::unique_ptr<spdlog::logger> inner_stderr_logger)
      : _spdlog_stdout_logger(std::move(inner_stdout_logger)), _spdlog_stderr_logger(std::move(inner_stderr_logger))
  {
    _spdlog_stdout_logger->set_pattern(details::default_pattern);
    _spdlog_stdout_logger->set_level(spdlog::level::info);
    _spdlog_stderr_logger->set_pattern(details::default_pattern);
    _spdlog_stderr_logger->set_level(spdlog::level::info);
  }

  template <typename... Args>
  void err_info(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void err_warn(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void err_error(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void err_critical(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    // we ignore max_limit with critical log
    if (_location == output_location::compat) { _spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...); }
    else if (_location == output_location::err)
    {
      _spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...);
    }
    else
    {
      _spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void out_info(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->info(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->info(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void out_warn(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->warn(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->warn(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void out_error(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    if (_log_count <= _max_limit)
    {
      if (_location == output_location::compat) { _spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...); }
      else if (_location == output_location::err)
      {
        _spdlog_stderr_logger->error(fmt, std::forward<Args>(args)...);
      }
      else
      {
        _spdlog_stdout_logger->error(fmt, std::forward<Args>(args)...);
      }
    }
  }

  template <typename... Args>
  void out_critical(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _log_count++;
    // we ignore max_limit with critical log
    if (_location == output_location::compat) { _spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...); }
    else if (_location == output_location::err)
    {
      _spdlog_stderr_logger->critical(fmt, std::forward<Args>(args)...);
    }
    else
    {
      _spdlog_stdout_logger->critical(fmt, std::forward<Args>(args)...);
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

struct logger
{
private:
  std::shared_ptr<details::logger_impl> _logger_impl;

  logger(std::shared_ptr<details::logger_impl> inner_logger) : _logger_impl(std::move(inner_logger)) {}

  friend logger create_default_logger();
  friend logger create_null_logger();
  friend logger create_custom_sink_logger(void* context, logger_output_func_t func);
  friend logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func);

public:
  template <typename... Args>
  void err_info(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->err_info(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void err_warn(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->err_warn(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void err_error(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->err_error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void err_critical(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->err_critical(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void out_info(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->out_info(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void out_warn(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->out_warn(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void out_error(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->out_error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void out_critical(const fmt::format_string<Args...>& fmt, Args&&... args)
  {
    _logger_impl->out_critical(fmt, std::forward<Args>(args)...);
  }

  void set_level(log_level lvl);
  void set_max_output(size_t max);
  void set_location(output_location location);
  size_t get_log_count() const;
  void log_summary();
};

inline logger create_default_logger()
{
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  auto stderr_sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
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
