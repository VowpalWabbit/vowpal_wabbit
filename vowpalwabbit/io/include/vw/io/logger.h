// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/common/string_view.h"

#include <fmt/core.h>

#include <functional>
#include <memory>

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
  TRACE_LEVEL = 0,
  DEBUG_LEVEL = 1,
  INFO_LEVEL = 2,
  WARN_LEVEL = 3,
  ERROR_LEVEL = 4,
  CRITICAL_LEVEL = 5,
  OFF_LEVEL = 6
};

log_level get_log_level(const std::string& level);

using logger_output_func_t = void (*)(void*, VW::io::log_level, const std::string&);
using logger_legacy_output_func_t = void (*)(void*, const std::string&);

namespace details
{
class log_sink
{
public:
  virtual ~log_sink() = default;
  virtual void info(const std::string&) = 0;
  virtual void warn(const std::string&) = 0;
  virtual void error(const std::string&) = 0;
  virtual void critical(const std::string&) = 0;
  virtual void set_level(log_level) = 0;
};

class logger_impl
{
public:
  std::unique_ptr<log_sink> stdout_log_sink;
  std::unique_ptr<log_sink> stderr_log_sink;
  size_t max_limit = SIZE_MAX;
  size_t log_count = 0;
  output_location location = output_location::COMPAT;

  logger_impl(std::unique_ptr<log_sink> inner_stdout_logger, std::unique_ptr<log_sink> inner_stderr_logger);

  void err_info(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stderr_log_sink->info(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->info(message); }
      else { stdout_log_sink->info(message); }
    }
  }

  void err_warn(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stderr_log_sink->warn(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->warn(message); }
      else { stdout_log_sink->warn(message); }
    }
  }

  void err_error(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stderr_log_sink->error(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->error(message); }
      else { stdout_log_sink->error(message); }
    }
  }

  void err_critical(const std::string& message)
  {
    log_count++;
    // we ignore max_limit with critical log
    if (location == output_location::COMPAT) { stderr_log_sink->critical(message); }
    else if (location == output_location::STDERR) { stderr_log_sink->critical(message); }
    else { stdout_log_sink->critical(message); }
  }

  void out_info(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stdout_log_sink->info(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->info(message); }
      else { stdout_log_sink->info(message); }
    }
  }

  void out_warn(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stdout_log_sink->warn(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->warn(message); }
      else { stdout_log_sink->warn(message); }
    }
  }

  void out_error(const std::string& message)
  {
    log_count++;
    if (log_count <= max_limit)
    {
      if (location == output_location::COMPAT) { stdout_log_sink->error(message); }
      else if (location == output_location::STDERR) { stderr_log_sink->error(message); }
      else { stdout_log_sink->error(message); }
    }
  }

  void out_critical(const std::string& message)
  {
    log_count++;
    // we ignore max_limit with critical log
    if (location == output_location::COMPAT) { stdout_log_sink->critical(message); }
    else if (location == output_location::STDERR) { stderr_log_sink->critical(message); }
    else { stdout_log_sink->critical(message); }
  }
};

}  // namespace details

class logger
{
public:
  logger(const logger&) = default;
  logger& operator=(const logger&) = default;
  logger(logger&&) noexcept = default;
  logger& operator=(logger&&) noexcept = default;

#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_info(fmt::format(fmt, std::forward<Args>(args)...));
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_warn(fmt::format(fmt, std::forward<Args>(args)...));
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_error(fmt::format(fmt, std::forward<Args>(args)...));
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void err_critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void err_critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->err_critical(fmt::format(fmt, std::forward<Args>(args)...));
  }
#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_info(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_warn(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_error(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void out_critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void out_critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_critical(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void info(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void info(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_info(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void warn(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void warn(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_warn(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void error(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void error(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_error(fmt::format(fmt, std::forward<Args>(args)...));
  }

#if FMT_VERSION >= 80000
  template <typename... Args>
  void critical(fmt::format_string<Args...> fmt, Args&&... args)
#else
  template <typename FormatString, typename... Args>
  void critical(const FormatString& fmt, Args&&... args)
#endif
  {
    _logger_impl->out_critical(fmt::format(fmt, std::forward<Args>(args)...));
  }

  void set_level(log_level lvl);
  void set_max_output(size_t max);
  void set_location(output_location location);
  size_t get_log_count() const;
  void log_summary();

private:
  std::shared_ptr<details::logger_impl> _logger_impl;

  logger(std::shared_ptr<details::logger_impl> inner_logger);

  friend logger create_default_logger();
  friend logger create_null_logger();
  friend logger create_custom_sink_logger(void* context, logger_output_func_t func);
  friend logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func);
};

logger create_default_logger();
logger create_null_logger();
logger create_custom_sink_logger(void* context, logger_output_func_t func);
logger create_custom_sink_logger_legacy(void* context, logger_legacy_output_func_t func);
}  // namespace io
}  // namespace VW
