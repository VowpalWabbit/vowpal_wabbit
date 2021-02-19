// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <memory>
#include <utility>
#include <spdlog/spdlog.h>

namespace VW
{
namespace io
{
  /*
  // TODO: should this be an object to be passed around or stand-alone 
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
  enum class log_level { trace, debug, info, warn, error, critical, off };

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

  void log_set_level(log_level lvl);
}
}
}
