// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include <string>
#include <utility>

#include <spdlog/spdlog.h>
// needed for custom types (like string_view)
#include <fmt/ostream.h>

namespace VW
{
namespace io
{
  /*
  // TODO: this be an object thats passed around, not stand-alone functions 
struct logger {
private:
  std::shared_ptr<spdlog::logger> _internal_logger;
public:
  vw_logger()
  : _internal_logger(spdlog::default_logger()) {}
  
  template<typename FormatString, typename... Args>
    void log_info(const FormatString &fmt, Args&&...args)
  {
    _internal_logger->info(fmt, std::forward<Args>(args)...);
  }

  template<typename FormatString, typename... Args>
    void log_error(const FormatString &fmt, Args&&...args)
  {
    _internal_logger->error(fmt, std::forward<Args>(args)...);
  }


  template<typename FormatString, typename... Args>
    void log_critical(const FormatString &fmt, Args&&...args)
  {
    _internal_logger->critical(fmt, std::forward<Args>(args)...);
  }

};
  */

namespace logger
{
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

  // do we need the rest of the levels?
  template<typename FormatString, typename... Args>
    void log_info(const FormatString &fmt, Args&&...args)
  {
    spdlog::default_logger_raw()->info(fmt, std::forward<Args>(args)...);
  }

  template<typename FormatString, typename... Args>
    void log_warn(const FormatString &fmt, Args&&...args)
  {
    spdlog::default_logger_raw()->warn(fmt, std::forward<Args>(args)...);
  }
  
  template<typename FormatString, typename... Args>
    void log_error(const FormatString &fmt, Args&&...args)
  {
    spdlog::default_logger_raw()->error(fmt, std::forward<Args>(args)...);
  }

  template<typename FormatString, typename... Args>
    void log_critical(const FormatString &fmt, Args&&...args)
  {
    spdlog::default_logger_raw()->critical(fmt, std::forward<Args>(args)...);
  }


  // FIXME: the get() call returns a shared_ptr. Keep a copy here to avoid unnecessary shared_ptr copies
  // This can go away once we move to an object-based logger
  namespace detail
  {
    extern std::shared_ptr<spdlog::logger> _stderr_logger;
  }
  
  // These should go away once we move to an object-based logger
  // do we need the rest of the levels?
  template<typename FormatString, typename... Args>
    void errlog_info(const FormatString &fmt, Args&&...args)
  {
    detail::_stderr_logger->info(fmt, std::forward<Args>(args)...);
  }

  template<typename FormatString, typename... Args>
    void errlog_warn(const FormatString &fmt, Args&&...args)
  {
    detail::_stderr_logger->warn(fmt, std::forward<Args>(args)...);
  }
  
  template<typename FormatString, typename... Args>
    void errlog_error(const FormatString &fmt, Args&&...args)
  {
    detail::_stderr_logger->error(fmt, std::forward<Args>(args)...);
  }

  template<typename FormatString, typename... Args>
    void errlog_critical(const FormatString &fmt, Args&&...args)
  {
    detail::_stderr_logger->critical(fmt, std::forward<Args>(args)...);
  }


  // Ensure modifications to the header info are localized
  class pattern_guard {
  public:
    pattern_guard(const std::string& pattern);
    ~pattern_guard();
  };
  void log_set_level(log_level lvl);

  void initialize_logger();
}
}
}
