// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "logger.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace VW
{
namespace io
{
namespace logger
{
// FIXME: the get() call returns a shared_ptr. Keep a copy here to avoid unnecessary shared_ptr copies
// This can go away once we move to an object-based logger
namespace detail{
  std::shared_ptr<spdlog::logger> _stderr_logger = spdlog::stderr_logger_st("stderr");
  const constexpr char* default_pattern = "[%l] %v";
}
  
void log_set_level(log_level lvl)
{
    spdlog::level::level_enum spdlog_lvl = spdlog::get_level();
    switch(lvl) {
    case log_level::trace:
      spdlog_lvl = spdlog::level::trace;
      break;
    case log_level::debug:
      spdlog_lvl = spdlog::level::debug;
      break;
    case log_level::info:
      spdlog_lvl = spdlog::level::info;
      break;
    case log_level::warn:
      spdlog_lvl = spdlog::level::warn;
      break;
    case log_level::error:
      spdlog_lvl = spdlog::level::err;
      break;
    case log_level::critical:
      spdlog_lvl = spdlog::level::critical;
      break;
    case log_level::off:
      spdlog_lvl = spdlog::level::off;
      break;
    }

    spdlog::set_level(spdlog_lvl);
}

pattern_guard::pattern_guard(const std::string& pattern)
{
  spdlog::set_pattern(pattern);
}

pattern_guard::~pattern_guard()
{
  spdlog::set_pattern(detail::default_pattern);
}
  
void initialize_logger()
{
  spdlog::set_pattern(detail::default_pattern);
}
  
}
}
}
